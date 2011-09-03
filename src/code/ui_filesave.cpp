
#include "ui_filesave.h"
#include "util_readwrite.h"

// checks and gets name for for something to be saved as, if valid name is provided, otherwise returns false
bool save_as_name( std::string & myname, bool tap_file, bool wav_file )
{
    DirScanner dScan;
	if( tap_file && wav_file )
	    dScan.setFileTypes( "Tapestrea Template Files (*.tap;*.xml)\0*.tap;*.xml\0Tapestrea Template XML Files (*.xml)\0*.xml\0Tapestrea Template TAP files (*.tap)\0*.tap\0Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0" );
	else if( tap_file )
		dScan.setFileTypes( "Tapestrea Template Files (*.tap;*.xml)\0*.tap;*.xml\0Tapestrea Template XML Files (*.xml)\0*.xml\0Tapestrea Template TAP files (*.tap)\0*.tap\0All Files (*.*)\0*.*\0" );
	else if( wav_file )
		dScan.setFileTypes( "Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0" );
	else
		dScan.setFileTypes( "All Files (*.*)\0*.*\0" );
    fileData * fD = dScan.saveFileDialog();
    if ( fD )
    {
#ifndef __TAPS_XML_ENABLE__
		if( fD->fileName.rfind( ".xml" ) == fD->fileName.length() - 4 ) {
			msg_box("xml disabled", "XML taps files have not been enabled in this version");
			return false;
		}
#endif
		if( fD->fileName.rfind( ".tap" ) == fD->fileName.length() - 4 || 
			fD->fileName.rfind( ".wav" ) == fD->fileName.length() - 4 || 
			fD->fileName.rfind( ".xml" ) == fD->fileName.length() - 4 )
        {
            BB_log( BB_LOG_INFO, "saving %s", fD->fileName.c_str() );
            const char * c = fD->fileName.c_str();
            myname = c;
            return true;
        }
        else
        {
            msg_box( "nice try", "only .tap, .wav or .xml files are allowed here" );
        }
    }
    return false;
}

// saves template to given filename
bool save_to_file( Template * temp, std::string fname, void * anal_info )
{   
    if( fname.length() == 0 )
        return false; 

    if( fname.rfind(".tap") == fname.length()-4 || fname.rfind(".xml") == fname.length()-4 ) 
    {
        // write template file
        TemplateWriter w;
        if( w.open( (char *)(fname.c_str()) ) )
        {
            w.write_template( temp, anal_info ); 
            w.close();
            return true;
        }
        else
        {
            msg_box( "ding!", "cannot open template file for writing!" );
        }
    }
    else if( fname.rfind(".wav") == fname.length()-4 )
    {
        // write sound file (later)
        TemplateGrabber tg( 0 );
        if( temp->type != TT_RESIDUE )
        {
            Frame grabframe;
            tg.grab( temp, &grabframe );
            SNDFILE * sf;
            SF_INFO sf_info;
            sf_info.samplerate = BirdBrain::srate();
            sf_info.channels = 1;
            sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//            sf = sf_open( fname.c_str(), SFM_WRITE, &sf_info );
			sf = AudioCentral::instance()->m_cachemanager->opensf( fname.c_str(), SFM_WRITE, &sf_info );
            if( sf )
            {
                sf_writef_float( sf, grabframe.waveform, grabframe.wsize );
//                sf_close( sf );
				AudioCentral::instance()->m_cachemanager->closesf( sf );
                return true;
            }
            else
            {
                msg_box( "ding!", "cannot open .wav file for writing!" );
            }
        }
    }

    return false;
}
