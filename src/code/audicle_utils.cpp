/*----------------------------------------------------------------------------
    TAPESTREA: Techniques And Paradigms for Expressive Synthesis, 
               Transformation, and Rendering of Environmental Audio
      Engine and User Interface

    Copyright (c) 2006 Ananya Misra, Perry R. Cook, and Ge Wang.
      http://taps.cs.princeton.edu/
      http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: audicle_utils.cpp
// desc: i don't know
//
// authors: Philip Davidson (philipd@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "audicle_utils.h"
#include "audicle_gfx.h"
#include "taps_birdbrain.h"

#ifdef __PLATFORM_LINUX__
#include <gtk/gtk.h>
#endif


fileData * openfD = NULL;
fileData * savefD = NULL;


fileData * 
fileData::clone() { 
    fileData * clon = new fileData();
    *clon = *this;
    if ( next ) { 
        clon->next = next->clone();
    }
    return clon;
}
void
listworkingdir(void) { 
        
   fprintf(stderr, "current directory ( . ) \n" ) ;

#ifdef __PLATFORM_WIN32__
   WIN32_FIND_DATA _file;
   HANDLE          _hFile;
   if ( (_hFile = FindFirstFile( ".", &_file)) != INVALID_HANDLE_VALUE ) { 
       do { 
           fprintf(stderr, "\t- %s \n", _file.cFileName);
       } while ( FindNextFile(_hFile, &_file) ); 
       FindClose(_hFile);
   }
#else
   DIR * dr = opendir(".");
   if ( dr ) { 
       struct dirent* d;
       while ( d = readdir(dr) ) { 
           fprintf(stderr, "\t- %s \n", d->d_name );
       }
       closedir(dr);
   }
#endif
   fprintf(stderr, "end directory\n");
}

DirScanner::DirScanner() {
   _imp = PlatformFactory::Instance()->DirScan();
}

void  
DirScanner::defineSearch(char * directory, char * pattern)  
{ 
   _imp->defineSearch(directory, pattern);  
} 

bool  
DirScanner::hasFile()                                       
{ 
   return _imp->hasFile();                  
}

void  
DirScanner::getFile(fileData * fD)
{ 
   _imp->getFile(fD);
} 

fileData * 
DirScanner::saveFileDialog() { 
//    fprintf(stderr, "DirScanner - dispatch\n");
    return _imp->saveFileDialog();
}

fileData * 
DirScanner::openFileDialog() { 
//    fprintf(stderr, "DirScanner - dispatch\n");
    return _imp->openFileDialog();
}

void
DirScanner::setFileTypes( const char * ft ) {
    _imp->m_types = ft;
}

#ifdef __PLATFORM_WIN32__

void
winDirScanner::defineSearch( char * path, char * pattern ) { 
   strcpy ( _patbuf, path );
   strcat ( _patbuf, "\\");
   strcat ( _patbuf, pattern );
   _hasFile = false;
   if ( (_hFile = FindFirstFile( _patbuf, &_file)) != INVALID_HANDLE_VALUE )
      _hasFile =true;
}

bool
winDirScanner::hasFile() { 
   return (_hasFile == 0 );
}

void
winDirScanner::getFile(fileData * fD) { 
    if ( !_hasFile ) 
        return;

    fD->fileName = _file.cFileName;
    fD->isDir = (_file.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY ) ? true : false ;

    if ( !(_hasFile = FindNextFile(_hFile, &_file)) ) 
        FindClose(_hFile);
}



fileData* 
winDirScanner::openFileDialog() { 

    // last
    //if( BirdBrain::our_last_open_dir != "" )
    //  BirdBrain::goto_dir( BirdBrain::our_last_open_dir );

    char newname[512];
    newname[0] = '\0';
    
    if ( openfD ) { 
        delete openfD;
        openfD = NULL;
    }
    //XXX make this non-modal 
    BB_log( BB_LOG_INFO, "opening win32 open file dialog..." );
    BB_pushlog();
    
    ShowCursor(true);
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lpstrFile = newname;
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = m_types ? m_types : "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = newname;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";
    if( BirdBrain::our_last_open_dir != "" )
        ofn.lpstrInitialDir = BirdBrain::our_last_open_dir.c_str(); 
    
    if( GetOpenFileName( &ofn ) && ofn.lpstrFile )
    {
        // BB_log( BB_LOG_INFO, "file: %s %s", ofn.lpstrFile, ofn.lpstrFileTitle );
        openfD = new fileData();
        fileData * newFile = openfD;
        char * next = newname; 
        next += strlen(next) + 1;
        if( *next )
        { 
            char basepath[512];
            char fullpath[512];
            strncpy( basepath, newname, 512 );
            strcat( basepath, "\\" );
            do
            { 
                strcpy( fullpath, basepath );
                strcat( fullpath, next );
                // fprintf( stderr, "returned multi file %s\n", fullpath );
                newFile->fileName = fullpath;
                next += strlen(next) + 1;
                if( *next )
                { 
                    newFile->next = new fileData();
                    newFile = newFile->next;
                }
            }while ( *next ) ;
        } 
        else
        {   //no extra files, we copied the full path in already
            // BB_log( BB_LOG_INFO, "got single file: %s", newname );
            openfD->fileName = newname;
        }
    }
    else
    {
        if( CommDlgExtendedError() == 0 )
        {
            BB_log( BB_LOG_INFO, "no file selected..." );
        }
        else
        {
            BB_log( BB_LOG_INFO, "openfile error: %d", CommDlgExtendedError() );
        }

        BB_poplog();

        return NULL;
    }

    ShowCursor(false);

    // save
    BirdBrain::our_last_open_dir = BirdBrain::getpath( openfD->fileName.c_str() );

    BB_poplog();
    
    return openfD;
}

fileData* 
winDirScanner::saveFileDialog() { 

    if ( savefD ) { 
        delete savefD;
        savefD = NULL;
    }

    // last
    //if( BirdBrain::our_last_save_dir != "" )
    //  BirdBrain::goto_dir( BirdBrain::our_last_save_dir );

    BB_log( BB_LOG_INFO, "opening win32 save file dialog" );
    char newname[512];
    newname[0] = '\0';
    ShowCursor(true);
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lpstrFile = newname;
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = m_types ? m_types : "Tapestrea Template Files (*.tap)\0*.tap\0Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = newname;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";
    if( BirdBrain::our_last_save_dir != "" )
        ofn.lpstrInitialDir = BirdBrain::our_last_save_dir.c_str();
    
    if( GetSaveFileName( &ofn ) ) { 
        strncpy ( newname, ofn.lpstrFile, 512 );
        savefD = new fileData();
        savefD->fileName = newname;
    }
    else { 
        return NULL;
    }

    ShowCursor(false);

    // save
    BirdBrain::our_last_save_dir = BirdBrain::getpath( savefD->fileName.c_str() );

    return savefD;

}

#else

void
UnixDirScanner::defineSearch ( char * path, char * pattern ) {
    strcpy ( _pathbuf, path ) ;
    if ( pattern ) strcpy ( _patternbuf, pattern ) ;
    fprintf (stderr, "scanner : searching '%s' for '%s'\n", path, pattern );
    fprintf (stderr, " current directory : %s\n", getenv ("PWD") );
    if ( pattern && strlen (pattern) > 0 ) {
        char * s = strrchr (_patternbuf, '*');
        if ( s )    strcpy ( _suffix, s+1 );
        else    strcpy ( _suffix , _patternbuf );
        _hasPat = true;
    }
    else _hasPat = false;
    
    if ( (_dir = opendir(_pathbuf)) == NULL ) {
        fprintf(stderr, "directory open failed (%s)\n", _pathbuf);
        _hasFile = false;
        return;
    }
    fprintf(stderr, "open succeeded\n");
    while ( (_ent = readdir ( _dir )) != NULL && ( _hasPat && !strstr (_ent->d_name, _suffix) ) ) { printf("skipped %s\n", _ent->d_name); }
    
    if ( _ent ) _hasFile = true;
    else closedir(_dir);
    
}


#ifdef __PLATFORM_LINUX__

// the file selector
GtkWidget * g_file_selector = NULL;
const char * g_gtk_filename;
std::string g_gtk_filename2;
t_TAPBOOL g_gtk_lock = FALSE;

// store filename
void store_filename( GtkFileSelection * selector, gpointer user_data )
{
    // get file name
    g_gtk_filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(g_file_selector) );
    g_gtk_filename2 = g_gtk_filename;
    // unlock
    g_gtk_lock = FALSE;

    // destroy
    gtk_widget_destroy( g_file_selector );
    // stop it
    gtk_main_quit();    
}

// cancel
void cb_cancel( GtkFileSelection * selector, gpointer user_data )
{
    // unlock
    g_gtk_lock = FALSE;
    // destroy
    gtk_widget_destroy( g_file_selector );
    // stop it
    gtk_main_quit();
}

// bye
void cb_linux_is_hard( GtkFileSelection * selector, gpointer user_data )
{
    // destroy
    gtk_widget_destroy( g_file_selector );
    g_gtk_lock = FALSE;
    // stop it
    gtk_main_quit();
}

// create a file selection box
void create_file_selection()
{
    // file select  
    g_file_selector = gtk_file_selection_new( "Please select a file" );

    // connect to something
    gtk_signal_connect( GTK_OBJECT 
                        (GTK_FILE_SELECTION(g_file_selector)->ok_button),
                        "clicked",
                        GTK_SIGNAL_FUNC(store_filename), 
                        NULL
                      );

    // connect to something
    gtk_signal_connect( GTK_OBJECT 
                        (GTK_FILE_SELECTION(g_file_selector)->cancel_button),
                        "clicked",
                        GTK_SIGNAL_FUNC(cb_cancel), 
                        NULL
                      );
/*
    // Ensure that the dialog box is destroyed when the user clicks a button.
    gtk_signal_connect_object( GTK_OBJECT 
                               (GTK_FILE_SELECTION(g_file_selector)->ok_button),
                               "clicked",
                               GTK_SIGNAL_FUNC(gtk_widget_destroy),
                               (GtkObject *)g_file_selector
                             );

    // connect to something
    gtk_signal_connect_object( GTK_OBJECT 
                               (GTK_FILE_SELECTION(g_file_selector)->cancel_button),
                               "clicked",
                               GTK_SIGNAL_FUNC(gtk_widget_destroy),
                               (GtkObject *)g_file_selector
                             );
*/

    gtk_signal_connect( GTK_OBJECT
                        (g_file_selector),
            "destroy",
            GTK_SIGNAL_FUNC(cb_linux_is_hard),
            NULL
              );
 
    // lock
    g_gtk_lock = TRUE;
    // reset
    g_gtk_filename = NULL;
    // display dialog box
    gtk_window_set_position( (GtkWindow *)g_file_selector, GTK_WIN_POS_CENTER );
    gtk_widget_show_all( g_file_selector );
    
    // wait
    gtk_main();
}

#endif


fileData* 
UnixDirScanner::openFileDialog()
{
  if( openfD ) {
    delete openfD;
    openfD = NULL;
  }

#ifndef __PLATFORM_LINUX__
  char newfilebuf[512];
  fprintf( stderr, "\nopen file : \n" );
  scanf( "%s", newfilebuf );
  
  if( strlen( newfilebuf ) )
  {
      openfD = new fileData();
      openfD->fileName = newfilebuf;
      openfD->isDir = false;
  }
#else
  // if locked
  if( g_gtk_lock ) return NULL;
  
  // last
  if( BirdBrain::our_last_open_dir != "" )
      BirdBrain::goto_dir( BirdBrain::our_last_open_dir );

  // go
  create_file_selection();
  
  // wait
  while( g_gtk_lock ) usleep( 30000 );

  if( g_gtk_filename ) {
    openfD = new fileData();
    openfD->fileName = g_gtk_filename2;
    openfD->isDir = false;
    
    // save
    BirdBrain::our_last_open_dir = BirdBrain::getpath( openfD->fileName.c_str() );
  }
#endif
  
  return openfD;
} 
 
fileData* 
UnixDirScanner::saveFileDialog() 
{ 
  if( savefD ) {
    delete savefD;
    savefD = NULL;
  }
  
#ifndef __PLATFORM_LINUX__
  char newfilebuf[512];
  fprintf( stderr, "\nopen file :\n" );
  scanf( "%s", newfilebuf );
  
  if( strlen( newfilebuf ) )
  {
      openfD = new fileData();
      openfD->fileName = newfilebuf;
      openfD->isDir = false;
  }
#else
  // if locked
  if( g_gtk_lock ) return NULL;
  
  // last
  if( BirdBrain::our_last_save_dir != "" )
      BirdBrain::goto_dir( BirdBrain::our_last_save_dir );

  // go
  create_file_selection();
  
  // wait
  while( g_gtk_lock ) usleep( 30000 );
  
  if( g_gtk_filename ) { 
    savefD = new fileData();
    savefD->fileName = g_gtk_filename2; 
    savefD->isDir = false;
    
    // save
    BirdBrain::our_last_save_dir = BirdBrain::getpath( savefD->fileName.c_str() );
  }
#endif

  return savefD;
}

bool
UnixDirScanner::hasFile() {
    return _hasFile;
}

void
UnixDirScanner::getFile( fileData * fD ) {

    if ( !_hasFile ) return;
    
    fD->fileName = _ent->d_name;
    stat ( _ent->d_name, &filestat );
    // fD->isDir = ( IS_DIR(filestat.st_mode) ) ? true : false ;
    fD->isDir = false;

    while( (_ent = readdir ( _dir )) != NULL && ( _hasPat && !strstr (_ent->d_name, _suffix) ) ) { printf("skipped %s\n", _ent->d_name); }

    if ( !_ent ) { 
        _hasFile = false;
        closedir ( _dir ) ;
    }    
}

#ifdef __PLATFORM_MACOSX__

#include <Carbon/Carbon.h>

NavEventUPP gNavEventHandlerPtr;

void userActionCallback ( NavCBRecPtr callBackParms )
{
    OSStatus err;
    long int n;
    NavReplyRecord * nReply = new NavReplyRecord;
    err = NavDialogGetReply ( callBackParms->context, nReply );
    if ( err != noErr ) {
        NavDisposeReply( nReply );
        return;
    } 

    NavUserAction userAction; 
    userAction = NavDialogGetUserAction( callBackParms->context );

    switch ( userAction )
    {
    case kNavUserActionOpen:
        AECountItems( &(nReply->selection), &n ) ;
        if ( n != 0 ) { 
            AEKeyword aeKey;
            AEDesc record;
            FSRef fref;
            char newfilename[512];
            openfD = new fileData();
            fileData * newfD = openfD;
            for ( int i = 0; i < n ; i++ ) { 
                if ( i != 0 ) { 
                    newfD->next = new fileData();
                    newfD = newfD->next;
                }                

                AEGetNthDesc (&(nReply->selection), i+1, typeFSRef, &aeKey, &record );
                err = AEGetDescData( &record, ( void * )( &fref ), sizeof( FSRef ) );      
                FSRefMakePath( &fref, (UInt8*)newfilename, 512);
                // fprintf(stderr, "open: %s\n", newfilename );
                newfD->fileName = newfilename;
                newfD->isDir = false;
            }
        }

        break;
    case kNavUserActionSaveAs:
        AECountItems( &(nReply->selection), &n ) ;
        if ( n != 0 ) { 
            AEKeyword aeKey;
            AEDesc record;
            FSRef fref;
            char newfilename[512];
            char fileSaveName[512];
            savefD = new fileData();
            for ( int i = 0; i < n ; i++ ) { 
                AEGetNthDesc (&(nReply->selection), i+1, typeFSRef, &aeKey, &record );
                err = AEGetDescData( &record, ( void * )( &fref ), sizeof( FSRef ) );      
                FSRefMakePath( &fref, (UInt8*)newfilename, 512);
                // fprintf(stderr, "save: %s\n", newfilename );
                savefD->fileName = newfilename;
                savefD->isDir = false;

                const char * fname  = CFStringGetCStringPtr( nReply->saveFileName, kCFStringEncodingASCII );
                if ( fname ) {
                    // fprintf(stderr, "file is %s\n", fname);
                    savefD->fileName += "/";
                    savefD->fileName += fname;
                }
                else { 
                    CFStringGetCString ( nReply->saveFileName, (char*)fileSaveName, 512, kCFStringEncodingASCII );
                    savefD->fileName += "/";
                    savefD->fileName += fileSaveName;
                    // fprintf(stderr, "no filename given\n");
                }

            }
        }
        break;
    }
    NavDisposeReply( nReply );
}


pascal void dialogFileCallback (
    NavEventCallbackMessage callBackSelector, 
    NavCBRecPtr  callBackParams, 
    void*   callBackUD )
{ 
    switch ( callBackSelector )
    {
    case kNavCBUserAction:
        userActionCallback( callBackParams );
        break;
    case kNavCBTerminate:
        NavDialogDispose( callBackParams->context );
        DisposeNavEventUPP( gNavEventHandlerPtr );
        break;
    }
}

fileData * MacOSXDirScanner::openFileDialog()
{
    OSStatus   err;
    NavDialogRef  openDialog;
    NavDialogCreationOptions dialogAttributes;
    
    int n = 0;

    if( openfD ) { 
        delete openfD;
        openfD = NULL;
    }

    if( AudicleWindow::our_fullscreen )
    {
        glutReshapeWindow( AudicleWindow::main()->m_w, AudicleWindow::main()->m_h );
        glutPostRedisplay();
        AudicleWindow::our_fullscreen = FALSE;
        return NULL;
    }

    // last
    // if( BirdBrain::our_last_open_dir != "" )
    //     BirdBrain::goto_dir( BirdBrain::our_last_open_dir );

    err = NavGetDefaultDialogCreationOptions( &dialogAttributes );
    
    dialogAttributes.modality = kWindowModalityAppModal;   
    
    gNavEventHandlerPtr = NewNavEventUPP( dialogFileCallback );    
    
    err = NavCreateGetFileDialog( &dialogAttributes, NULL, 
                                  gNavEventHandlerPtr, NULL, NULL, 
                                  NULL, &openDialog );
    
    if ( !AudicleGfx::cursor_on ) { ShowCursor(); AudicleGfx::cursor_on = true; }
    err = NavDialogRun( openDialog );
    
    if ( err != noErr )
    {
        NavDialogDispose( openDialog );
        DisposeNavEventUPP( gNavEventHandlerPtr );
        return NULL;
    }

    return openfD;
}

fileData * MacOSXDirScanner::saveFileDialog()
{
    //XXX not finished yet. 

    BB_log( BB_LOG_INFO, "opening OSX file save dialog" );
    OSStatus   err;
    NavDialogRef  saveDialog;

    NavDialogCreationOptions dialogAttributes;
        
    int n = 0;
    if ( savefD ) { 
        delete savefD;
        savefD = NULL;
    }

    if( AudicleWindow::our_fullscreen )
    {
        glutReshapeWindow( AudicleWindow::main()->m_w, AudicleWindow::main()->m_h );
        glutPostRedisplay();
        AudicleWindow::our_fullscreen = FALSE;
        return NULL;
    }

    err = NavGetDefaultDialogCreationOptions( &dialogAttributes );
    
    dialogAttributes.modality = kWindowModalityAppModal;   
    
    gNavEventHandlerPtr = NewNavEventUPP( dialogFileCallback );    
    
    err = NavCreatePutFileDialog( &dialogAttributes, 'ChuK', kNavGenericSignature,
                                  gNavEventHandlerPtr, NULL, &saveDialog );
    
    if ( !AudicleGfx::cursor_on ) { ShowCursor(); AudicleGfx::cursor_on = true; }
    err = NavDialogRun( saveDialog );
    
    if ( err != noErr )
    {
        NavDialogDispose( saveDialog );
        DisposeNavEventUPP( gNavEventHandlerPtr );
        return NULL;
    }
    
    return savefD;
}

#endif

#endif


PlatformFactory* PlatformFactory::_instance = NULL;

PlatformFactory*
PlatformFactory::Instance() { 

   if ( !_instance ) 
      _instance = new PlatformFactory();

   return _instance;

}

DirScannerImp*
PlatformFactory::DirScan() { 

#ifdef __PLATFORM_WIN32__
//    fprintf(stderr, "windows dir scanner\n");
    return new winDirScanner();
#else
#ifdef __PLATFORM_MACOSX__
//    fprintf(stderr, "mac dir scanner\n");
    return new MacOSXDirScanner();
#else
//it's he     fprintf(stderr, "*nix dir scanner\n");
    return new UnixDirScanner();
#endif
#endif

}

double interp (int a, int b, double w ) { 
   return (double)a*(1.0-w) + (double)b*w;
}

double interp ( double a, double b, double w ) { 
   return a*(1.0-w) + b*w;
}

int fgetline(char**buf, int *size, FILE *fp)
{
  int len;
  int c;

  if (fp == NULL)
    return -1;

  if (*buf == NULL) {
    if (*size == 0) 
      *size = BUFSIZ;
    
    if ((*buf = (char*)malloc(*size)) == NULL)
      return -1;
  }

  if (feof(fp))
    return -1;
    
  len = 0;
  while ((c = getc(fp)) != EOF && c != '\n') {
    if (len+1 >= *size) {
      if ((*buf = (char*)realloc(*buf, *size += BUFSIZ)) == NULL) return -1;
    }
    (*buf)[len++] = c;
  }

  if (len == 0 && c == EOF)
    return -1;
    
  (*buf)[len] = '\0';
    
  return len;

} /* fgetline */


void
fmBuffer::netrecv(char * addr, int port ) { 
   //empty
   fprintf(stderr, "fmBuffer::netrecv : no imp!\n");
}

void
fmBuffer::netsend(char * addr, int port ) { 
   //empty
   fprintf(stderr, "fmBuffer::netsend : no imp!\n");
}

int
fmBuffer::readfile(char* m) { 
   if ( _mode != FMODE_FREAD) return 1;
   char * s = new char[256];
   int sz = 256;
   _buffer.erase();
   FILE * f = fopen(m , "r");
   std::string nl = "\n";
   if ( f ) { 
      while ( fgetline(&s, &sz, f) != EOF ) {   
         _buffer += s;
         _buffer += nl;
      }
      _ready = true;
      _pos = std::string::npos;
      fclose(f);
      return 0;
   }
   else return 1;
}


void
fmBuffer::writefile(char*m) { 
   
   if ( _mode != FMODE_FWRITE ) return;
   flush();
   FILE * f = fopen(m , "w");
   if ( f ) { 
      fprintf(f, "%s", _buffer.c_str() );
      fclose(f);
   }
}

int
fmBuffer::getline(std::string &s) { 

   if ( !_ready ) return EOF;
   if ( _pos == (int)std::string::npos  ) _pos = 0; 
   int end = _buffer.find("\n", _pos);

   if ( end == (int)std::string::npos) { 
      _ready = false;
      s = _buffer.substr(_pos);
      _pos = std::string::npos;
   }
   else { 
      int len = end - _pos;
      s = _buffer.substr(_pos, len);
      _pos = end+1;
   }
   return s.length();

}



int
fmBuffer::getline(char** sc, int * sz) { 

   if ( !_ready ) return EOF;
   
   if ( _pos == (int)std::string::npos  ) _pos = 0; 
   
   int end = _buffer.find("\n", _pos);

   int len = ( end == (int)std::string::npos ) ? _buffer.length() - _pos : end - _pos;
   
   if ( len > *sz ) { 
      if ( ( *sc = (char*)realloc( *sc, len+1 )) == NULL ) { 
         fprintf(stderr,"reallocfailed\n");
         return EOF;
      }
   }

   if ( end == (int)std::string::npos) { 
      _ready = false;
      strcpy(*sc, _buffer.substr(_pos).c_str() );
      _pos = std::string::npos;
   }
   else { 
      strcpy(*sc, _buffer.substr(_pos, len).c_str());
      (*sc)[len] = '\0';
      _pos = end+1;
   }

   return len;

}

void
fmBuffer::flush() { 
   _buffer += _line;
   _line[0] = '\0';
}

char*
fmBuffer::in() { 
   flush(); // line into buffer   
   return (char*) _line; 
}


int
tokenize(char * buf, char *tok, char***args, int *size) { 
   //buf must persist after this function is called!
   int i =0;
   char * t = strtok(buf, tok);
   while ( t != NULL ) { 
      while ( i+1 > *size ) { 
         *size *= 2; 
         *args = (char**) realloc(*args, *size * sizeof(char*) ); 
      }
      (*args)[i] = t;
      i++;
      t = strtok(NULL, tok);
   }
   return i;  
}

