/*
  Demo for class FatLib by Jean-Michel Gallego
  FatLib allow you to easily switch between libraries SdFat and FatFs
    by defining identifier FAT_USE_SDFAT at beginning of file FatLib.h

    SdFat is an Arduino library written by William Greiman
    that provides read/write access to FAT16/FAT32
    file systems on SD/SDHC flash cards.
    
    FatFs module is a generic FAT file system for
    small embedded systems developed by ChaN.
    I wrote a class to wrap FatFs on Arduino
*/

#include <SPI.h>
#include <FatLib.h>

// Modify according to your hardware
// #define MY_SD_CS_PIN 4     // Chip Select for SD card reader with Ethernet shield
// #define MY_SD_CS_PIN 53    // Chip Select for SD card reader on Due
   #define MY_SD_CS_PIN 15    // Default Chip Select for SD card reader on Esp8266

void setup()
{
  int res;

  // If other chips are connected to SPI bus,
  //   set to high the pin connected to their CS
  /*
  pinMode( 10, OUTPUT ); 
  digitalWrite( 10, HIGH );
  pinMode( 4, OUTPUT ); 
  digitalWrite( 4, HIGH );
  */

  Serial.begin( 115200 );
  do
    delay( 10 );
  while( Serial.available() && Serial.read() >= 0 );
  Serial.print( "Testing FatLib with library " );
  #if FAT_USE == FAT_SDFAT
    Serial.println( "SdFat" );
  #else
    Serial.println( "FatFs" );
  #endif
  Serial.println( "Press a key to start" );
  while( Serial.read() < 0 )
    delay( 1 );
  delay( 400 );
  Serial.println();

  uint32_t millisBeginTest = millis();

  // Mount SD card
  Serial.print( "Mount the SD card:  " );
  res = FAT_FS.begin( MY_SD_CS_PIN, SPI_FULL_SPEED );
  printError( res, "Unable to mount SD card" );
  Serial.println( "ok" );

  // Show capacity and free space of SD card
  Serial.print( "Capacity of card:   " );
  Serial.print( FAT_FS.capacity());
  Serial.println( " kBytes" );
  Serial.print( "Free space on card: " );
  Serial.print( FAT_FS.free());
  Serial.println( " kBytes" );

  // List root directory
  Serial.println();
  Serial.println( "List of directories and files in root:" );
  FAT_DIR dir;
  if( dir.openDir( "/" ))
  {
    while( dir.nextFile())
    {
      Serial.print( dir.fileName());
      Serial.print( "\t" );
      if( dir.isDir() )
        Serial.println( "(Directory)" );
      else
      {
        Serial.print( dir.fileSize());
        Serial.println( " Bytes" );
      }
    }
    dir.closeDir();
  }
  
  // Create directory
  char * dirName = "/New Directory";
  Serial.println();
  Serial.print( "Create directory '" );
  Serial.print( dirName );
  Serial.println( "'" );
  if( FAT_FS.exists( dirName ))
  {
    Serial.print( dirName );
    Serial.println( " already exists!" );
  }
  else
  {
    res = FAT_FS.mkdir( dirName );
    printError( res, "Error creating directory" );
  }

  // Create a file in that directory
  FAT_FILE file;
  char * fileName = "/New Directory/A new file.txt";
  Serial.println();
  Serial.print( "Create file '" );
  Serial.print( fileName );
  Serial.println( "'" );
  res = file.open( fileName, O_RDWR | O_CREAT );
  printError( res, "Error creating file" );
  
  // Writing text to a file and closing it
  Serial.println();
  Serial.println( "Write to file" );
  res = file.writeString( "Test d'ecriture dans un fichier\r\n" );
  if( res >= 0 )
    res = file.writeString( "Testing writing to file\r\n" );
  if( res >= 0 )
    res = file.writeString( "Prueba de escritura en un archivo\r\n" );
  // Write next line using writeBuffer()
  if( res >= 0 )
  {
    char * ps1 = "Test di scrittura su file\r\n";
    uint32_t nwrite = file.write( ps1, strlen( ps1 ));
    res = nwrite == strlen( ps1 );
  }
  // Write last line byte per byte,  using writeChar()
  if( res >= 0 )
  {
    char * ps2 = "Testes de gravacao em um arquivo\r\n";
    uint8_t pc = 0 ;
    while( res > 0 && ps2[ pc ] != 0 )
      res = file.writeChar( ps2[ pc ++ ] );
  }
  printError( ( res >= 0 ), "Error writing to file" );

  // Read content of file
  Serial.println();
  Serial.print( "Content of '" );
  Serial.print( fileName );
  Serial.println( "' is:" );
  char line[ 64 ];
  file.seekSet( 0 ); // set cursor to beginning of file
  while( file.readString( line, sizeof( line )) >= 0 )
    Serial.println( line );

  // Close the file
  Serial.println();
  Serial.println( "Close the file" );
  res = file.close();
  printError( res, "Error closing file" );

  // Rename and move the file to roor
  char * newName = "/TEST.TXT";
  Serial.println();
  Serial.print( "Rename file '" );
  Serial.print( fileName );
  Serial.print( "' to '" );
  Serial.print( newName );
  Serial.println( "'" );
  if( FAT_FS.exists( newName ))
    FAT_FS.remove( newName );
  res = FAT_FS.rename( fileName, newName );
  printError( res, "Error renaming file" );

  // Show content of directories
  Serial.println();
  Serial.print( "Content of '" );
  Serial.print( dirName );
  Serial.println( "' must now be empty:" );
  listDir( dirName );
  Serial.println();
  listDir( "/" );

  // Open file for reading
  Serial.println();
  Serial.print( "Open file '" );
  Serial.print( newName );
  Serial.println( "'" );
  res = file.open( newName, O_READ );
  printError( res, "Error opening file" );

  // Read content of file and close it
  Serial.println();
  Serial.print( "Content of '" );
  Serial.print( newName );
  Serial.println( "' is:" );
  // Read first line byte per byte, just to demostrate use of readChar()
  char c;
  do
    Serial.print( c = file.readChar());
  while( c != '\n' && c != 0 );
  // Read next lines with readString() and print length of lines
  int l;
  while( ( l = file.readString( line, sizeof( line ))) >= 0 )
  {
    Serial.print( line );
    Serial.print( " (Length is: " );
    Serial.print( l );
    Serial.println( ")" );
  }
  file.close();
    
  // Delete the file
  Serial.println();
  Serial.println( "Delete the file" );
  res = FAT_FS.remove( newName );
  printError( res, "Error deleting file" );

  // Delete the directory
  Serial.println();
  Serial.print( "Delete directory '" );
  Serial.print( dirName );
  Serial.println( "'" );
  res = FAT_FS.rmdir( dirName );
  printError( res, "Error deleting directory" );

  Serial.println();
  Serial.print( "Test ok in " );
  Serial.print( millis() - millisBeginTest );
  Serial.println( " milliseconds" );
}

void loop()
{
}

//    LIST DIRECTORY

void listDir( char * dirname )
{
  FAT_DIR d;
  
  Serial.print( "\nContent of directory '" );
  Serial.print( dirname );
  Serial.println( "' :" );
  if( d.openDir( dirname ))
  {
    uint8_t entrees = 0;
    while( d.nextFile())
    {
      Serial.println( d.fileName());
      entrees ++;
    }
    d.closeDir();
    Serial.print( entrees );
    Serial.print( " files or directories in " );
    Serial.println( dirname );
  }
}

//    PRINT ERROR STRING & STOP EXECUTION

// if ok is false, print the string msg and enter a while loop for ever

void printError( int ok, char * msg )
{
  if( ok )
    return;
  Serial.print( msg );
  Serial.print( ": " );
  #if FAT_USE == FAT_SDFAT
    /*
    // Have to make cardErrorCode() and cardErrorData() public in SdFat.h
    //  in order to compile next 3 lines
    Serial.print( FAT_FS.cardErrorCode());
    Serial.print( " - " );
    Serial.print( FAT_FS.cardErrorData());
    */
    FAT_FS.errorPrint(); // only send result if cardErrorCode() non zero
  #else
    Serial.println( FAT_FS.error());
  #endif
  Serial.println();
  while( true )
    delay( 1 );
}