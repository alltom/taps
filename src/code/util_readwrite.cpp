#include "util_readwrite.h"
#include "util_base64.h"
#include "util_thread.h"
#include "util_network.h"

// macros for XMLWriter
#define RETURN_IF_ERROR(x)	{if((x) == false) return 0;}
#define MY_ENCODING "ISO-8859-1"

#ifdef __TAPS_XML_ENABLE__
// debugging function
void debug(xmlNodePtr cur, int level=0) {
	while(cur != NULL) {
		for(int i = 0; i < level; i++)
			fprintf(stderr, "  ");
		fprintf(stderr, "%s\n", cur->name);
		debug(cur->xmlChildrenNode, level+1);
		cur = cur->next;
	}
}

// XML DOCUMENT BUILDER

XMLBuilder::XMLBuilder() {
	the_bufsize = 1024;
	doc = NULL;
	root = NULL;
	cur = NULL;
}

XMLBuilder::~XMLBuilder() {
	// always take copy (if for posterity) so that doc can be freed?
	xmlFreeDoc(doc); 
}

bool XMLBuilder::start_document(const char * rootstring) {
	doc = xmlNewDoc(NULL); 
	if(doc == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:start_document: Error creating xml document tree"); 
		return false;
	}
	root = xmlNewNode(NULL, BAD_CAST rootstring); 
	if(root == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:start_document: Error creating root node");
		return false;
	}
	xmlDocSetRootElement(doc, root); 
	if(xmlDocGetRootElement(doc) != root) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:start_document: Root check failed");
		return false;
	}
	cur = root;
	return true;
}

bool XMLBuilder::end_document() {
	cur = (xmlNodePtr)doc;
	BB_log(BB_LOG_INFO, "XMLBuilder:end_document reached"); 
	//debug(cur); 
	return true;
}

bool XMLBuilder::start_element(const char * name) {
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:start_element: cur is NULL"); 
		return false;
	}
	xmlNodePtr child = xmlNewTextChild(cur, NULL, BAD_CAST name, NULL); 
	if(child == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:start_element: child is NULL");
		return false;
	}
	cur = child;
	return true;
}

bool XMLBuilder::end_element() {
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:end_element: cur is NULL"); 
		return false;
	}
	if(cur->parent == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:end_element: parent is NULL");
		return false;
	}
	cur = cur->parent;
	return true;
}

bool XMLBuilder::add_element(const char * name, const char * content) {
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_element: cur is NULL"); 
		return false;
	}
	xmlNodePtr child = xmlNewTextChild(cur, NULL, BAD_CAST name, BAD_CAST content); 
	if(child == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_element: child is NULL");
		return false;
	}
	// cur remains at parent node
	return true; 
}

bool XMLBuilder::add_attribute(const char * name, const char * value) {
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_attribute: cur is NULL"); 
		return false;
	}
	xmlAttrPtr attr = xmlNewProp(cur, BAD_CAST name, BAD_CAST value); 
	if(attr == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_attribute: attribute is NULL");
		return false;
	}
	// cur remains at parent node
	return true;
}

bool XMLBuilder::add_branch(xmlNodePtr branch) {
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_branch: cur is NULL"); 
		return false;
	}
	if(branch == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_branch: branch is NULL");
	}
	xmlNodePtr child = xmlAddChild(cur, branch);  
	if(child == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:add_branch: failed");
		return false;
	}
	// cur remains at parent node
	return true;
}

bool XMLBuilder::format_element(const char * name, const char * format, ...) {
	memset(the_buffer, 0, the_bufsize);
	va_list ap;
	va_start(ap, format);
	vsprintf(the_buffer, format, ap);
	va_end(ap);
	if(!add_element(name, the_buffer)) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:format_element: failed");
		return false;
	}
	return true;
}

bool XMLBuilder::format_attribute(const char * name, const char * format, ...) {
	memset(the_buffer, 0, the_bufsize);
	va_list ap;
	va_start(ap, format);
	vsprintf(the_buffer, format, ap);
	va_end(ap);
	if(!add_attribute(name, the_buffer)) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:format_attribute: failed");
		return false;
	}
	return true;
}

xmlDocPtr XMLBuilder::get_doc() {
	return doc;
}

xmlDocPtr XMLBuilder::get_doc_copy() {
	if(doc == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLBuilder:get_doc_copy: doc is NULL");
		return NULL;
	}
	else
		return xmlCopyDoc(doc, 1); 
}

xmlDocPtr XMLBuilder::copy(xmlDocPtr orig) {
	return xmlCopyDoc(orig, 1); // recursive copy
}
#endif


// TEMPLATE WRITER
bool TemplateWriter::open(char *filename) {
	tw = NULL;
	std::string fn = std::string(filename);
	if(fn.rfind(".tap") == fn.length()-4)
		tw = new TapWriter();
#ifdef __TAPS_XML_ENABLE__
	else if(fn.rfind(".xml") == fn.length()-4)
		tw = new XMLWriter();
#endif
	if(tw != NULL) 
		return tw->open(filename); 
	BB_log(BB_LOG_WARNING, "TemplateWriter:open: could not save TAPESTREA template in file %s", filename);
}

bool TemplateWriter::close() {
	if(tw != NULL) {
		bool ret = tw->close(); 
		SAFE_DELETE(tw);
		tw = NULL;
		return ret;
	}
	return false;
}

int TemplateWriter::write_template(const Template * tempt, void * analinfo) {
	if(tw != NULL) {
		int ret = tw->write_template(tempt, analinfo); 
		return ret;
	}
	BB_log(BB_LOG_WARNING, "TemplateWriter:write_template: could not write"); 
	return 0;
}


// XML WRITER
#ifdef __TAPS_XML_ENABLE__
// open text writer
bool XMLWriter::open(char * fileName) {
	filename = std::string(fileName);
	BB_log(BB_LOG_INFO, "XMLWriter:open: Created XMLWriter for file %s", fileName);
	return true;
}

// close
bool XMLWriter::close() {
	// save file (builder should free its own doc)
	xmlDocPtr doc = builder.get_doc();
	if(doc == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLWriter:close: Doc is NULL");
		return false;
	}
	int written = xmlSaveFormatFile(filename.c_str(), doc, 1);
	if(written < 0) {
		BB_log(BB_LOG_SEVERE, "XMLWriter:close: Error writing to file %s", filename.c_str());
		return false;
	}
	BB_log(BB_LOG_INFO, "XMLWriter:close: %d bytes written to file %s", written, filename.c_str()); 
	return true;
}

// write a template
int XMLWriter::write_template(const Template * tempt, void * analinfo) {
	RETURN_IF_ERROR(builder.start_document("template")); 
	int ret = store_template(tempt, analinfo); 
	RETURN_IF_ERROR(builder.end_document()); // end template
	return ret;	
}

// store template info: assume template tag has been started and will 
// be ended by invoking function
int XMLWriter::store_template(const Template * tempt, void * analinfo) {
	RETURN_IF_ERROR(builder.format_attribute("type", "%d", tempt->type));
	RETURN_IF_ERROR(builder.format_attribute("id", "%d", (t_TAPINT)tempt->id)); 
	RETURN_IF_ERROR(builder.add_attribute("name", tempt->name.c_str())); 
	RETURN_IF_ERROR(builder.add_attribute("typestr", tempt->type_str())); 
	// storage info
	int ret = 0;
	RETURN_IF_ERROR(builder.start_element("storageinfo")); 
	if(tempt->type == TT_DETERMINISTIC)
		ret = store_det((Deterministic *)tempt);
	else if(tempt->type == TT_RAW) 
		ret = store_raw((Raw *)tempt);
	else if(tempt->type == TT_TRANSIENT)
		ret = store_trans((Transient *)tempt);
	else if(tempt->type == TT_RESIDUE)
		ret = store_res((Residue *)tempt);
	else if(tempt->type == TT_LOOP)
		ret = store_loop((LoopTemplate *)tempt); 
	else if(tempt->type == TT_TIMELINE)
		ret = store_tl((Timeline *)tempt);
	else if(tempt->type == TT_BAG)
		ret = store_bag((BagTemplate *)tempt); 
	else if(tempt->type == TT_FILE) ;
	else if(tempt->type == TT_SCRIPT) ;
	RETURN_IF_ERROR(builder.end_element()); // end storage info 
	// synth info
	RETURN_IF_ERROR(builder.start_element("synthesisinfo"));
	RETURN_IF_ERROR(builder.format_element("gain", "%f", tempt->gain));
	RETURN_IF_ERROR(builder.format_element("pan", "%f", tempt->pan));
	RETURN_IF_ERROR(builder.format_element("time_stretch", "%f", tempt->time_stretch)); 
	RETURN_IF_ERROR(builder.format_element("freq_warp", "%f", tempt->freq_warp)); 
	RETURN_IF_ERROR(builder.format_element("periodicity", "%f", tempt->periodicity));
	RETURN_IF_ERROR(builder.format_element("density", "%f", tempt->density));
	RETURN_IF_ERROR(builder.end_element()); // end synth info
	// add analysis info
	if(analinfo != NULL) {
		xmlDocPtr anal = (xmlDocPtr)analinfo;
		xmlNodePtr analroot = xmlDocGetRootElement(anal);
		if(!builder.add_branch(analroot)) {
			BB_log(BB_LOG_SEVERE, "XMLWriter:write_template: could not add analysis info");
			RETURN_IF_ERROR(builder.end_element()); // end template
			return 0;
		}
	}
	return ret;
}

// write sinusoidal storage info
int XMLWriter::store_det(const Deterministic * debt) {
	RETURN_IF_ERROR(builder.start_element("detstorage"));
	Track * t;
	int track_size;
	freqpolar * fp;
	int numtracks = debt->tracks.size();
	RETURN_IF_ERROR(builder.format_element("numtracks", "%d", numtracks)); 
	// tracks
	for(int i = 0; i < numtracks; i++) {
		t = debt->tracks[i]; 
		track_size = t->history.size();
		RETURN_IF_ERROR(builder.start_element("track")); 
		RETURN_IF_ERROR(builder.format_attribute("size", "%d", track_size));
		RETURN_IF_ERROR(builder.format_attribute("id", "%d", t->id));
		RETURN_IF_ERROR(builder.format_attribute("state", "%d", t->state));
		RETURN_IF_ERROR(builder.format_attribute("start", "%f", t->start));
		RETURN_IF_ERROR(builder.format_attribute("end", "%f", t->end));
		RETURN_IF_ERROR(builder.format_attribute("phase", "%f", t->phase));
		RETURN_IF_ERROR(builder.format_attribute("historian", "%d", t->historian));
		// history points
		for(int j = 0; j < t->history.size(); j++) {
			fp = &(t->history[j]); 
			RETURN_IF_ERROR(builder.start_element("history"));
			RETURN_IF_ERROR(builder.format_attribute("index", "%d", j));
			RETURN_IF_ERROR(builder.format_attribute("freq", "%f", fp->freq));
			RETURN_IF_ERROR(builder.format_attribute("mag", "%f", fp->p.mag));
			RETURN_IF_ERROR(builder.format_attribute("phase", "%f", fp->p.phase));
			RETURN_IF_ERROR(builder.format_attribute("time", "%f", fp->time));
			RETURN_IF_ERROR(builder.format_attribute("bin", "%u", fp->bin));
			RETURN_IF_ERROR(builder.format_attribute("ismatched", "%i", fp->isMatched));  
			RETURN_IF_ERROR(builder.end_element()); // end history
		}
		RETURN_IF_ERROR(builder.end_element()); // end track
	}
	RETURN_IF_ERROR(builder.end_element()); // end detstorage
	return numtracks;
}

// write transient storage info
int XMLWriter::store_trans(const Transient * trance) {
	RETURN_IF_ERROR(builder.start_element("transtorage"));
	int ret = store_frame(trance->sound); 
	RETURN_IF_ERROR(builder.end_element()); // end transtorage
	return ret;
}

// write raw storage info
int XMLWriter::store_raw(const Raw * war) {
	RETURN_IF_ERROR(builder.start_element("rawstorage"));
	int ret = store_frame(war->sound); 
	RETURN_IF_ERROR(builder.end_element()); // end rawstorage
	return ret;
}

// write a frame
int XMLWriter::store_frame(const Frame &frame) {
	RETURN_IF_ERROR(builder.start_element("frame"));
	RETURN_IF_ERROR(builder.format_attribute("wlen", "%u", frame.wlen));
	RETURN_IF_ERROR(builder.format_attribute("wsize", "%u", frame.wsize));
	RETURN_IF_ERROR(builder.format_attribute("time", "%d", frame.time)); 
	// THIS IS ASSUMING float & t_TAPUINT have 4 bytes: MAY HAVE COMPATIBILITY ISSUES
	if(sizeof(float) != sizeof(t_TAPUINT)) {
		BB_log(BB_LOG_SEVERE, "XMLWriter:store_frame: size incompatibilities"); 
		return 0;
	}
	unsigned char * wf = new unsigned char[frame.wsize * sizeof(float)]; 
	memcpy(wf, frame.waveform, frame.wsize * sizeof(float));
	unsigned char * wfptr = wf;
	t_TAPUINT wtf = 0;
	for(int i = 0; i < frame.wsize; i++) {
		memcpy(&wtf, wfptr, sizeof(t_TAPUINT)); 
		wtf = htonl(wtf);
		memcpy(wfptr, &wtf, sizeof(t_TAPUINT));
		wfptr += sizeof(t_TAPUINT); 
	}
	std::string base64 = base64_encode(wf, (unsigned int)(frame.wsize * sizeof(float)));
	if(!builder.add_element("samples", base64.c_str())) {
		SAFE_DELETE_ARRAY(wf);
		return 0;
	}
	SAFE_DELETE_ARRAY(wf);
	RETURN_IF_ERROR(builder.end_element()); // end frame
}

// write residue storage info
int XMLWriter::store_res(const Residue * race) {
	TreesynthIO * io = race->tsio;
	Treesynth * tsth = race->ts;
	RETURN_IF_ERROR(builder.start_element("resstorage"));
	if(strcmp(race->filename, ""))
		RETURN_IF_ERROR(builder.add_element("filename", race->filename));
	RETURN_IF_ERROR(builder.start_element("wavelettree"));
	RETURN_IF_ERROR(builder.format_element("total_levels", "%d", race->total_levels)); 
	RETURN_IF_ERROR(builder.format_element("requested_levels", "%d", race->requested_levels)); 
	RETURN_IF_ERROR(builder.format_element("lefttree", "%d", (int)race->lefttree)); 
	if(strcmp(io->ifilename, ""))
		RETURN_IF_ERROR(builder.add_element("io_filename", io->ifilename)); 
	if(strcmp(io->leftfile, ""))
		RETURN_IF_ERROR(builder.add_element("io_leftfile", io->leftfile)); 
	if(strcmp(io->ofilename, ""))
		RETURN_IF_ERROR(builder.add_element("io_ofilename", io->ofilename)); 
	RETURN_IF_ERROR(builder.format_element("io_rm_mode", "%d", io->rm_mode)); 
	RETURN_IF_ERROR(builder.format_element("io_write_to_buffer", "%d", (int)io->write_to_buffer));
	RETURN_IF_ERROR(builder.format_element("io_write_to_file", "%d", (int)io->write_to_file)); 
	RETURN_IF_ERROR(builder.format_element("ts_percentage", "%f", tsth->percentage)); 
	RETURN_IF_ERROR(builder.format_element("ts_kfactor", "%f", tsth->kfactor)); 
	RETURN_IF_ERROR(builder.format_element("ts_stoplevel", "%d", tsth->stoplevel));
	RETURN_IF_ERROR(builder.format_element("ts_start_level", "%d", tsth->startlevel));
	RETURN_IF_ERROR(builder.format_element("ts_randflip", "%d", (int)tsth->randflip));
	RETURN_IF_ERROR(builder.format_element("ts_ancfirst", "%d", (int)tsth->ancfirst));
	RETURN_IF_ERROR(builder.end_element()); // end wavelettree
	RETURN_IF_ERROR(builder.start_element("olarandom"));
	if(strcmp(race->olar->ifilename, ""))
		RETURN_IF_ERROR(builder.add_element("io_filename", race->olar->ifilename));
	if(strcmp(race->olar->ofilename, ""))
		RETURN_IF_ERROR(builder.add_element("io_ofilename", race->olar->ofilename));
	RETURN_IF_ERROR(builder.format_element("io_write_to_buffer", "%d", (int)race->olar->write_to_buffer));
	RETURN_IF_ERROR(builder.format_element("io_write_to_file", "%d", (int)race->olar->write_to_file));
	RETURN_IF_ERROR(builder.format_element("randomness", "%f", race->olar->get_randomness()));
	RETURN_IF_ERROR(builder.format_element("segment_size", "%f", race->olar->get_segsize_secs()));
	RETURN_IF_ERROR(builder.format_element("minimum_distance", "%f", race->olar->get_mindist_secs()));
	RETURN_IF_ERROR(builder.format_element("scale_amplitude", "%d", race->olar->get_scaleamp()));
	RETURN_IF_ERROR(builder.end_element()); // end olarandom
	// could store the original file here as a frame
	RETURN_IF_ERROR(builder.end_element()); // end resstorage 
	return 1;
}

// write loop storage info
int XMLWriter::store_loop(const LoopTemplate * hole) {
	RETURN_IF_ERROR(builder.start_element("loopstorage")); 
	RETURN_IF_ERROR(builder.format_attribute("randomness", "%f", hole->random)); 
	RETURN_IF_ERROR(builder.start_element("event")); 
	RETURN_IF_ERROR(builder.start_element("template"));
	int ret = store_template(hole->temp, NULL); 
	RETURN_IF_ERROR(builder.end_element()); // end template
	RETURN_IF_ERROR(builder.end_element()); // end event
	RETURN_IF_ERROR(builder.end_element()); // end loopstorage
	return ret;
}

// write timeline storage info
int XMLWriter::store_tl(const Timeline * teal) {
	const Instance * inst;
	int num_instances = teal->instances.size();
	time_t inst_id;
	int ret = 1;
	RETURN_IF_ERROR(builder.start_element("tlstorage"));
	RETURN_IF_ERROR(builder.format_attribute("duration", "%f", teal->duration));
	RETURN_IF_ERROR(builder.format_attribute("num_instances", "%d", num_instances));
	RETURN_IF_ERROR(builder.format_attribute("start", "%f", teal->starttime));
	RETURN_IF_ERROR(builder.format_attribute("stop", "%f", teal->stoptime));
	RETURN_IF_ERROR(builder.format_attribute("loop", "%d", teal->loop)); 
	for(int i = 0; i < num_instances; i++) {
		inst = &(teal->instances[i]);
		inst_id = inst->ui_temp->core->id;
		RETURN_IF_ERROR(builder.start_element("instance"));
		RETURN_IF_ERROR(builder.format_element("id", "%d", (t_TAPINT)inst_id)); 
		RETURN_IF_ERROR(builder.format_element("start_time", "%f", (double)inst->start_time));
		RETURN_IF_ERROR(builder.format_element("end_time", "%f", (double)inst->end_time)); 
		RETURN_IF_ERROR(builder.format_element("y_offset", "%f", inst->y_offset));
		// include template only if it's not a timeline
		if(inst->ui_temp->core->type != TT_TIMELINE) {
			RETURN_IF_ERROR(builder.start_element("template"));
			ret = store_template(inst->ui_temp->core, NULL); 
			RETURN_IF_ERROR(builder.end_element()); // end template
		}
		RETURN_IF_ERROR(builder.end_element()); // end instance
	}
	RETURN_IF_ERROR(builder.end_element()); // end tlstorage
	return ret;
}

// write bag storage info
int XMLWriter::store_bag(const BagTemplate * bat) {
	const Marble * mab;
	int num_marbles = bat->marbles.size();
	time_t mab_id;
	int ret = 1;
	RETURN_IF_ERROR(builder.start_element("bagstorage")); 
	RETURN_IF_ERROR(builder.format_attribute("num_marbles", "%d", num_marbles)); 
	for(int i = 0; i < num_marbles; i++) {
		mab = &(bat->marbles[i]); 
		mab_id = mab->ui_temp->core->id;
		RETURN_IF_ERROR(builder.start_element("marble")); 
		RETURN_IF_ERROR(builder.format_element("id", "%d", (t_TAPINT)mab_id)); 
		RETURN_IF_ERROR(builder.format_element("play_once", "%d", (int)mab->playonce)); 
		RETURN_IF_ERROR(builder.format_element("likelihood", "%u", mab->likelihood)); 
		RETURN_IF_ERROR(builder.format_element("randomness", "%f", mab->random)); 
		// include template only if it's not a bagtemplate (which it shouldn't be)
		if(mab->ui_temp->core->type != TT_BAG) {
			RETURN_IF_ERROR(builder.start_element("template")); 
			ret = store_template(mab->ui_temp->core, NULL); 
			RETURN_IF_ERROR(builder.end_element()); // end template
		}
		RETURN_IF_ERROR(builder.end_element()); // end marble
	}
	RETURN_IF_ERROR(builder.end_element()); // end bagstorage
	return ret;
}
#endif

// TAP WRITER

bool TapWriter::open( char * filename )
{
    ofile.open( filename, ios::out );
    return ofile.good();
}

bool TapWriter::close()
{
    ofile.close();
	return true;
}

// templates
int TapWriter::write_template( const Template * tempt, void * analinfo )
{
    int ret;

    // make sure file is open
    if( ofile.bad() )
    {
        msg_box( "internal errorrr", "cannot write to no file." );
        return 0;
    }

    time_t myid = tempt->id; // get_id( tempt->id ); 

    ofile << tempt->type << " " << myid << std::endl;
    ofile << tempt->name << " " << tempt->type_str() << std::endl;
    ofile << tempt->time_stretch << " " << tempt->freq_warp << " " << tempt->gain << " "
          << tempt->pan << " " << tempt->periodicity << " " << tempt->density << std::endl;

/*  // not sure how this other method works (I mean it doesn't, but stuff should be changed for it to work)
    fprintf( ofile, "%i\n", tempt->type );
    fprintf( ofile, "%s %s\n", tempt->name, tempt->type_str() );
    fprintf( ofile, "%f %f %f %f %f %f\n", tempt->time_stretch, tempt->freq_warp, tempt->gain, 
             tempt->pan, tempt->periodicity, tempt->density );
*/

    switch( tempt->type )
    {
    case TT_DETERMINISTIC:
        ret = write_det( (Deterministic *) tempt );
        break;
    case TT_RAW:
        ret = write_raw( (Raw *) tempt );
        break;
    case TT_TRANSIENT:
        ret = write_trans( (Transient *) tempt );
        break;
    case TT_FILE:
        ret = write_file( (File *) tempt );
        break;
    case TT_RESIDUE:
        ret = write_res( (Residue *) tempt );
        break;
    case TT_LOOP:
        ret = write_loop( (LoopTemplate *) tempt );
        break;
    case TT_TIMELINE:
        ret = write_tl( (Timeline *) tempt );
        break;
    case TT_BAG:
        ret = write_bag( (BagTemplate *) tempt ); 
        break;
    default:
        ret = 0;
    }

    return ret;
}

int TapWriter::write_det( const Deterministic * debt )
{
    return write_tracks( debt->tracks ); 
}

int TapWriter::write_trans( const Transient * trance )
{
    return write_frame( trance->sound );
}

int TapWriter::write_raw( const Raw * war )
{
    return write_frame( war->sound );
}

int TapWriter::write_file( const File * phial )
{
    ofile << phial->filename << std::endl;
    return 1;
}

int TapWriter::write_res( const Residue * race )
{
    ofile << race->total_levels << " " << race->requested_levels << " " << race->lefttree << std::endl;
    TreesynthIO * io = race->tsio;
    int s;
    s = strcmp( io->ifilename, "" );
    ofile << (s != 0) << io->ifilename << std::endl;
    s = strcmp( io->leftfile, "" );
    ofile << (s != 0) << io->leftfile << std::endl;
    s = strcmp( io->ofilename, "" );
    ofile << (s != 0) << io->ofilename << std::endl;
    ofile << io->rm_mode << " " << io->write_to_buffer << " " << io->write_to_file << std::endl;
    Treesynth * tsth = race->ts;
    ofile << tsth->percentage << " " << tsth->kfactor << " " << tsth->stoplevel << " "
          << tsth->startlevel << " " << tsth->randflip << " " << tsth->ancfirst << std::endl;
    return 1; 
}

int TapWriter::write_loop( const LoopTemplate * hole )
{
    ofile << hole->random << std::endl;
    return write_template( hole->temp );
}

// idness is bad. needs to be unique somehow.
int TapWriter::write_tl( const Timeline * teal )
{
    const Instance * inst;
    int num_instances = teal->instances.size();
    time_t inst_id; 
    
    // write timeline and instance information
    ofile << teal->duration << " " << teal->loop << " " << num_instances << std::endl;
    for( int i = 0; i < num_instances; i++ )
    {
        inst = &(teal->instances[i]); 
        inst_id = inst->ui_temp->core->id; //get_id( inst->ui_temp->core->id ); 
        ofile << inst_id << " " << inst->start_time << " " << inst->end_time << " " 
              << inst->y_offset << std::endl;
    }

    // write instances themselves (as long as they're not timelines, for now) (to prevent infinite writing)
    for( int j = 0; j < num_instances; j++ )
    {
        inst = &(teal->instances[j]);
        if( inst->ui_temp->core->type == TT_TIMELINE )
        {
            ofile << false << std::endl; 
        }
        else
        {
            ofile << true << std::endl;
            write_template( inst->ui_temp->core );
        }
    }

    return 1;
}

int TapWriter::write_bag( const BagTemplate * bat )
{
    const Marble * mab; 
    int num_marbles = bat->marbles.size();
    time_t mab_id; 
    
    // write marble information
    ofile << num_marbles << std::endl;
    for( int i = 0; i < num_marbles; i++ )
    {
        mab = &(bat->marbles[i]); 
        mab_id = mab->ui_temp->core->id; //get_id( inst->ui_temp->core->id ); 
        ofile << mab_id << " " << mab->playonce << " " << mab->likelihood << " " 
              << mab->random << std::endl;
    }

    // write marbles themselves (as long as they're not bagtemplates, which they shouldn't be)
    for( int j = 0; j < num_marbles; j++ )
    {
        mab = &(bat->marbles[j]); 
        if( mab->ui_temp->core->type == TT_BAG)
        {
            ofile << false << std::endl; 
        }
        else
        {
            ofile << true << std::endl;
            write_template( mab->ui_temp->core );
        }
    }

    return 1;
}

// other things (parts of templates)
int TapWriter::write_tracks( const std::vector<Track *> &event )
{
    int num_tracks = event.size();
    int track_size;
    Track * t;
    freqpolar * fp; 
    ofile << "num_tracks " << num_tracks << std::endl;
    for( int i = 0; i < num_tracks; i++ )
    {
        t = event[i]; 
        track_size = t->history.size(); 
        ofile << "track " << track_size << " " << t->id << " " << t->state << " "
              << t->start << " " << t->end << " " << t->phase << " " << t->historian << std::endl;
        for( int j = 0; j < track_size; j++ )
        {
            fp = &(t->history[j]); 
            ofile << fp->freq << " " << fp->p.mag << " " << fp->p.phase << " " << fp->time
                  << " " << fp->bin << " " << fp->isMatched << std::endl;
        }
    }
    return num_tracks;
}

// note: this only writes the waveform info, not the cmp/pol/freq stuff; may need to be added later
int TapWriter::write_frame( const Frame &frame )
{
    ofile << "frame " << frame.wlen << " " << frame.wsize << " " << frame.time << std::endl;
    for( int i = 0; i < frame.wsize; i++ )
    {
        ofile << frame.waveform[i] << " ";
    }
    ofile << std::endl;
    return frame.wsize;
}


// TEMPLATE READER
bool TemplateReader::open(char *filename) {
	tr = NULL;
	std::string fn = std::string(filename);
	if(fn.rfind(".tap") == fn.length()-4)
		tr = new TapReader();
#ifdef __TAPS_XML_ENABLE__
	else if(fn.rfind(".xml") == fn.length()-4)
		tr = new XMLReader();
#endif
	if(tr != NULL) 
		return tr->open(filename); 
	BB_log(BB_LOG_WARNING, "TemplateReader:open: could not open file %s as a TAPESTREA template", filename);
}

void TemplateReader::close() {
	if(tr != NULL) {
		tr->close(); 
		SAFE_DELETE(tr);
		tr = NULL;
	}
}

Template * TemplateReader::read_template() {
	if(tr != NULL) {
		return tr->read_template();
	}
	BB_log(BB_LOG_WARNING, "TemplateReader:read_template: could not read"); 
	return NULL;
}


// XML READER
#ifdef __TAPS_XML_ENABLE__
// open and verify
bool XMLReader::open(char * filename) {
	doc = xmlParseFile(filename);
	if(doc == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:open: Document %s not parsed successfully", filename);
		return false;
	}
	root = xmlDocGetRootElement(doc);
	if(root == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:open: Document %s is empty", filename);
		xmlFreeDoc(doc);
		return false;
	}
	if(!name_match(root, "template")) {
		BB_log(BB_LOG_SEVERE, "XMLReader::open: Document %s is of the wrong type; root node is %s instead of \'template\'", filename, root->name);
		xmlFreeDoc(doc);
		return false;
	}
	
	ns = root->ns;
	//debug(root); 
	return true;
}

// close file
void XMLReader::close() {
	if(doc != NULL) {
		xmlFreeDoc(doc); // frees xml tree
	}
	//xmlCleanupParser(); // use only when you are done using libxml2... so never?
	//xmlMemoryDump(); // to debug memory for regression tests
}

// read a template
Template * XMLReader::read_template() {
fprintf(stderr, "read_template\n"); 
	// if root is not as it should be:
	if(!name_match(root, "template")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_template: wrong node name");
		return NULL;
	}
	return parse_template(root);
}

// parse a template: assume curroot is already at an element named "template"
Template * XMLReader::parse_template(xmlNodePtr curroot) {
fprintf(stderr, "parse_template\n");
	// read attributes: type, id, name, typestr
	int type;
	time_t id;
	char buf[50];
	std::string name, typestr;
	xmlChar *xml_type, *xml_id, *xml_name, *xml_typestr;
	xml_type = xmlGetProp(curroot, (const xmlChar *)"type");
	xml_id = xmlGetProp(curroot, (const xmlChar *)"id");
	xml_name = xmlGetProp(curroot, (const xmlChar *)"name");
	xml_typestr = xmlGetProp(curroot, (const xmlChar *)"typestr");
	id = (time_t)(atoi((char *)xml_id)); 
	type = atoi((char *)xml_type); 
	name = std::string((char *)xml_name);
	typestr = std::string((char *)xml_typestr); 
	xmlFree(xml_type); 
	xmlFree(xml_id);
	xmlFree(xml_name);
	xmlFree(xml_typestr); 
	Template * tempt = NULL; 
	// do the storage parsing first, to ensure that template is created
	xmlNodePtr cur = curroot->xmlChildrenNode;
	while(cur != NULL) {
		if(name_match(cur, "storageinfo")) {
			// tempt gets created (new) in one of these functions
			if(type == TT_DETERMINISTIC) 
				tempt = read_det(cur, id);
			else if(type == TT_RAW) 
				tempt = read_raw(cur, id);
			else if(type == TT_TRANSIENT)
				tempt = read_trans(cur, id);
			else if(type == TT_RESIDUE)
				tempt = read_res(cur, id);
			else if(type == TT_LOOP) 
				tempt = read_loop(cur, id);
			else if(type == TT_TIMELINE)
				tempt = read_tl(cur, id);
			else if(type == TT_BAG)
				tempt = read_bag(cur, id); 
			else if(type == TT_FILE) 
				tempt = NULL;
			else if(type == TT_SCRIPT)
				tempt = NULL;
			else
				tempt = NULL;
		}
		cur = cur->next;
	}
	if(tempt == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader: Could not read template %s of type %s", name.c_str(), typestr.c_str()); 
		return tempt;
	}
	// add basic info to tempt
	tempt->name = name;
	tempt->typestr = typestr;
	
	// now parse other info
	cur = curroot->xmlChildrenNode;
	while(cur != NULL) {
		if(name_match(cur, "analysisinfo")) {
			// not doing anything with analysis info now
		}
		else if(name_match(cur, "synthesisinfo")) {
			// read in synthesis info
			parse_synthesis_info(cur, tempt); 
		}
		cur = cur->next;
	}
	
	// done
	return tempt;
}

// parse generic synthesis info and update template with it
void XMLReader::parse_synthesis_info(xmlNodePtr cur, Template * tempt) {
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		read_double_if(cur, "gain", tempt->gain); 
		read_double_if(cur, "pan", tempt->pan); 
		read_double_if(cur, "time_stretch", tempt->time_stretch);
		read_double_if(cur, "freq_warp", tempt->freq_warp);
		read_double_if(cur, "periodicity", tempt->periodicity);
		read_double_if(cur, "density", tempt->density);
		cur = cur->next;
	}
}

// parse Deterministic template storage information
Deterministic * XMLReader::read_det(xmlNodePtr cur, time_t myid) {
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_det: no storage info available");
		return NULL;
	}
	bool matched = false;
	while(cur != NULL) {
		if(name_match(cur, "detstorage")) {
			matched = true;
			break;
		}
		cur = cur->next; 
	}
	if(!matched) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_det: storage type mismatch");
		return NULL;
	}
	std::vector<Track *> temp_tracks;
	Deterministic * debt;
	if(read_tracks(cur, temp_tracks)) 
		debt = new Deterministic(temp_tracks, myid);
	else
		debt = NULL;
	return debt;
}

// read tracks into event
int XMLReader::read_tracks(xmlNodePtr cur, std::vector<Track *> &event) {
	int num_tracks = 0;
	Track * t;
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		read_int_if(cur, "numtracks", num_tracks);
		if(name_match(cur, "track")) {
			t = parse_track(cur);
			if(t != NULL) 
				event.push_back(t);
		}
		cur = cur->next;
	}
	return num_tracks;
}

// parse a single track
Track * XMLReader::parse_track(xmlNodePtr cur) {
	int track_size;
	freqpolar temp_fp;
	Track * t = new Track;
	if(cur != NULL) {
		get_int_attribute(cur, "size", track_size);
		get_int_attribute(cur, "id", t->id);
		get_int_attribute(cur, "state", t->state);
		get_taptime_attribute(cur, "start", t->start); 
		get_taptime_attribute(cur, "end", t->end);
		get_double_attribute(cur, "phase", t->phase);
		get_tapuint_attribute(cur, "historian", t->historian);
	}
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:parse_track: no history info found");
		return NULL;
	}
	while(cur != NULL) {
		if(name_match(cur, "history")) {
			if(parse_history(cur, temp_fp)) 
				t->history.push_back(temp_fp);
		}
		cur = cur->next;
	}
	BB_log(BB_LOG_FINE, "Track info: size: %i, id: %i, state: %i, start: %f, end: %f, phase: %f, historian: %i\n", 
			track_size, t->id, t->state, t->start, t->end, t->phase, t->historian);
	return t;
}

// parse a history entry
bool XMLReader::parse_history(xmlNodePtr cur, freqpolar &temp_fp) {
	int hist_index = -1;
	if(cur != NULL) {
		get_int_attribute(cur, "index", hist_index);
		get_float_attribute(cur, "freq", temp_fp.freq); 
		get_float_attribute(cur, "mag", temp_fp.p.mag);
		get_float_attribute(cur, "phase", temp_fp.p.phase);
		get_taptime_attribute(cur, "time", temp_fp.time);
		get_tapuint_attribute(cur, "bin", temp_fp.bin);
		get_bool_attribute(cur, "ismatched", temp_fp.isMatched); 
		cur = cur->next;
	}
	BB_log(BB_LOG_FINE, "History info: index: %i, freq: %f, mag: %f, phase: %f, time, %f, bin: %i, matched: %i\n", 
			hist_index, temp_fp.freq, temp_fp.p.mag, temp_fp.p.phase, temp_fp.time, temp_fp.bin, temp_fp.isMatched); 
	return true;
}

// parse Transient template storage information
Transient * XMLReader::read_trans(xmlNodePtr cur, time_t myid) {
fprintf(stderr, "read_trans\n");
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_trans: no storage info available");
		return NULL;
	}
	if(!find_element(&cur, "transtorage")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_trans: storage type mismatch");
		return NULL;
	}
	Frame f;
	Transient * trance;
	if(read_frame(cur, f)) 
		trance = new Transient(f, myid);
	else
		trance = NULL;
	return trance;
}

// parse Raw template storage information
Raw * XMLReader::read_raw(xmlNodePtr cur, time_t myid) {
fprintf(stderr, "read_raw\n");
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_raw: no storage info available");
		return NULL;
	}
	if(!find_element(&cur, "rawstorage")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_raw: storage type mismatch");
		return NULL;
	}
	Frame f;
	Raw * war;
	if(read_frame(cur, f)) 
		war = new Raw(f, myid);
	else
		war = NULL;
	return war;
}

// read a frame
int XMLReader::read_frame(xmlNodePtr cur, Frame &frame) {
fprintf(stderr, "read_frame\n");	
	cur = cur->xmlChildrenNode; 
	while(cur != NULL) {
		if(name_match(cur, "frame")) {
			t_TAPUINT wlen, wsize;
			get_tapuint_attribute(cur, "wlen", wlen);
			get_tapuint_attribute(cur, "wsize", wsize);
			get_taptime_attribute(cur, "time", frame.time);
			frame.alloc_waveform(wlen);
			frame.wsize = wsize;
			if(!read_frame_samples(cur, frame)) {
				BB_log(BB_LOG_SEVERE, "XMLReader:read_frame: could not read samples");
				return 0;
			}
			break;
		}
		cur = cur->next;
	}
	BB_log(BB_LOG_INFO, "Frame info: wlen: %u wsize: %u time: %f", frame.wlen, frame.wsize, frame.time); 
	return frame.wsize;
}

// read frame samples
bool XMLReader::read_frame_samples(xmlNodePtr cur, Frame &frame) {
fprintf(stderr, "read_frame_samples\n");
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_frame_samples: no samples found");
		return false;
	}
	while(cur != NULL) {
		if(name_match(cur, "samples")) {
			xmlChar * temp_xml = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); 
			std::string base64 = std::string((const char *)temp_xml); 
			xmlFree(temp_xml); 
			std::string bin = base64_decode(base64); 
			if(bin.length() != frame.wsize * sizeof(float)) {
				BB_log(BB_LOG_SEVERE, "XMLWriter:store_frame: size mismatch! bin.length: %d, frame.wsize * float: %d", bin.length(), frame.wsize * sizeof(float)); 
			}
			// THIS IS ASSUMING float & t_TAPUINT have 4 bytes: MAY HAVE COMPATIBILITY ISSUES
			if(sizeof(float) != sizeof(t_TAPUINT)) {
				BB_log(BB_LOG_SEVERE, "XMLWriter:store_frame: size incompatibilities"); 
				return false;
			}
			unsigned char * wf = new unsigned char[frame.wsize * sizeof(float)]; 
			memset(wf, 0, frame.wsize * sizeof(float)); 
			memcpy(wf, bin.c_str(), bin.length());
			unsigned char * wfptr = wf;
			t_TAPUINT wtf = 0;
			int max_i = bin.length() < frame.wsize ? bin.length() : frame.wsize; 
			for(int i = 0; i < max_i; i++) {
				memcpy(&wtf, wfptr, sizeof(t_TAPUINT)); 
				wtf = ntohl(wtf);
				memcpy(wfptr, &wtf, sizeof(t_TAPUINT));
				wfptr += sizeof(t_TAPUINT); 
			}
			memcpy(frame.waveform, wf, frame.wsize * sizeof(float)); 
			break;
		}
		cur = cur->next;
	}
	return true;
}

// parse Residue template storage information
Residue * XMLReader::read_res(xmlNodePtr cur, time_t myid) {
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res: no storage info available");
		return NULL;
	}
	if(!find_element(&cur, "resstorage")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res: storage type mismatch");
		return NULL;
	}
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res: no specific storage info available");
		return NULL;
	}
	// file name
	xmlNodePtr fname = cur;
	char filename[1024];
	if(find_element(&fname, "filename")) {
		read_string_if(fname, "filename", filename);
	}
	else if(!read_res_name(cur, filename)) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res: no file name found");
		return NULL;
	}
	Residue * race = new Residue(filename);
	bool wtree_found = false, olar_found = false;
	// treesynth
	xmlNodePtr wtree = cur;
	if(find_element(&wtree, "wavelettree")) {
		wtree_found = true;
		int total_levels = 0, requested_levels = 0; 
		bool lefttree = 0;
		TreesynthIO * io = race->tsio;
		Treesynth * tsth = race->ts; 
		wtree = wtree->xmlChildrenNode;
		while(wtree != NULL) {		
			read_int_if(wtree, "total_levels", total_levels);
			read_int_if(wtree, "requested_levels", requested_levels);
			read_bool_if(wtree, "lefttree", lefttree); 
			read_string_if(wtree, "io_filename", io->ifilename);
			read_string_if(wtree, "io_leftfile", io->leftfile);
			read_string_if(wtree, "io_ofilename", io->ofilename);
			read_int_if(wtree, "io_rm_mode", io->rm_mode);
			read_bool_if(wtree, "io_write_to_buffer", io->write_to_buffer);
			read_bool_if(wtree, "io_write_to_file", io->write_to_file);
			read_double_if(wtree, "ts_percentage", tsth->percentage);
			read_float_if(wtree, "ts_kfactor", tsth->kfactor);
			read_int_if(wtree, "ts_stoplevel", tsth->stoplevel);
			read_int_if(wtree, "ts_startlevel", tsth->startlevel);
			read_bool_if(wtree, "ts_randflip", tsth->randflip);
			read_bool_if(wtree, "ts_ancfirst", tsth->ancfirst);
			wtree = wtree->next;
		}
		if(total_levels != 0)
			tsth->tree->resetLevels(total_levels);
		else
			BB_log(BB_LOG_SEVERE, "XMLReader:read_res: wavelettree total_levels is 0");
	}
	// olar
	xmlNodePtr olar = cur;
	if(find_element(&olar, "olarandom")) {
		olar_found = true;
		olar = olar->xmlChildrenNode;
		float randomness = -1, mindist = -1, segsize = -1;
		bool scale_amp = false;
		while(olar != NULL) {
			read_string_if(olar, "io_filename", race->olar->ifilename);
			read_string_if(olar, "io_ofilename", race->olar->ofilename);
			read_bool_if(olar, "io_write_to_buffer", race->olar->write_to_buffer);
			read_bool_if(olar, "io_write_to_file", race->olar->write_to_file);
			read_float_if(olar, "randomness", randomness);
			read_bool_if(olar, "scale_amplitude", scale_amp);
			read_float_if(olar, "minimum_distance", mindist);
			read_float_if(olar, "segment_size", segsize);
			olar = olar->next;
		}
		// set params (for floats, won't change unless > 0)
		race->olar->set_randomness(randomness);
		race->olar->set_scaleamp(scale_amp);
		race->olar->set_mindist_secs(mindist);
		race->olar->set_segsize_secs(segsize);
		if(race->olar->get_segsize_secs() == 0)
			BB_log(BB_LOG_SEVERE, "XMLReader:read_res: olarandom segment size is 0");
	}
	// neither
	if(!wtree_found && !olar_found) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res: no wavelet tree or ola random info found");
		SAFE_DELETE(race);
		return NULL;
	}
	// done
	return race;
}	

// find ifilename from wavelettree information for older XML Residue
bool XMLReader::read_res_name(xmlNodePtr cur, char filename[]) {
	if(!find_element(&cur, "wavelettree")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res_name: no wavelettree info available");
		return false;
	}
	cur = cur->xmlChildrenNode;
	if(!find_element(&cur, "io_filename")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_res_name: no filename info in wavelettree");
		return false;
	}
	read_string_if(cur, "io_filename", filename);
	return true;
}	

// parse Loop template storage information
LoopTemplate * XMLReader::read_loop(xmlNodePtr cur, time_t myid) {
fprintf(stderr, "read_loop\n");
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_loop: no storage info available");
		return NULL;
	}
	if(!find_element(&cur, "loopstorage")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_loop: storage type mismatch");
		return NULL;
	}
	LoopTemplate * hole = NULL;
	Template * orig = NULL;
	// get randomness attribute
	double rand;
	get_double_attribute(cur, "randomness", rand); 
	// find 'event' element
	cur = cur->xmlChildrenNode;
	if(!find_element(&cur, "event")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_loop: cannot find event information"); 
		return NULL;
	}
	// find 'tempate' element
	cur = cur->xmlChildrenNode;
	if(!find_element(&cur, "template")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_loop: cannot find event template");
		return NULL;
	}
	// now 'template' has been found, read it!
	orig = parse_template(cur); 
	if(orig == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_loop: cannot parse event template");
		return NULL;
	}
	hole = new LoopTemplate(*orig, myid); 
	if(hole == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_loop: cannot create loop template");
		return NULL;
	}
	hole->random = rand;
	return hole;
}

// parse Timeline template storage information
Timeline * XMLReader::read_tl(xmlNodePtr cur, time_t myid) {
fprintf(stderr, "read_tl\n");
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_tl: no storage info available");
		return NULL;
	}
	if(!find_element(&cur, "tlstorage")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_tl: storage type mismatch");
		return NULL;
	}
	t_TAPTIME duration, starttime, stoptime;
	int loop, num_instances;
	get_taptime_attribute(cur, "duration", duration); 
	get_int_attribute(cur, "num_instances", num_instances);
	get_taptime_attribute(cur, "start", starttime);
	get_taptime_attribute(cur, "stop", stoptime);
	get_int_attribute(cur, "loop", loop); 
	Timeline * teal = new Timeline(duration, myid); 
	if(teal == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_tl: Could not create timeline");
		return NULL;
	}
	teal->starttime = starttime;
	teal->stoptime = stoptime;
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		if(name_match(cur, "instance")) {
			parse_instance(cur, teal); 
		}
		cur = cur->next;
	}
	return teal;
}

// parse a Timeline instance
void XMLReader::parse_instance(xmlNodePtr cur, Timeline * teal) {
fprintf(stderr, "parse_instance\n"); 
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:parse_instance: null pointer");
		return;
	}
	// get basics
	time_t id = 0;
	t_TAPTIME start = 0, end = 0;
	float y_offset = 0;
	long id_long = 0;
	xmlNodePtr next = cur->xmlChildrenNode; 
	while(next != NULL) {
		if(read_long_if(next, "id", id_long))
			id = (time_t)id_long;
		read_taptime_if(next, "start_time", start);
		read_taptime_if(next, "end_time", end); // not used
		read_float_if(next, "y_offset", y_offset);
		next = next->next;
	}
	// prepare to read template if appropriate
	UI_Template *uit = NULL, *dummy = NULL;
	Template *temp = NULL;
	// check whether template is already in library
	for(int k = 0; k < Library::instance()->size(); k++) {
		if(Library::instance()->templates[k]->core->id == id) { // jackpot
			uit = Library::instance()->templates[k];
			break;
		}
	}
	// if template does not exist in library
	if(uit == NULL) {
		// read template info from xml if available
		cur = cur->xmlChildrenNode; // update cur
		if(find_element(&cur, "template")) {
			temp = parse_template(cur); 
			if(temp != NULL) 
				uit = Library::instance()->add(temp); 
			else
				BB_log(BB_LOG_SEVERE, "XMLReader:parse_instance: could not read template"); 
		}
	}
	// now if we have a template, place it on the timeline in two steps
	if(uit != NULL) {
		// make dummy copy
		dummy = Library::instance()->add(uit->core); 
		uit->makedummy(dummy); 
		// place dummy on timeline
		teal->place(dummy, (double)start/teal->duration, y_offset); 
		BB_log(BB_LOG_INFO, "XMLReader:parse_instance: added template %s to timeline", uit->core->name.c_str());
	}
}

// parse Bag template storage information
BagTemplate * XMLReader::read_bag(xmlNodePtr cur, time_t myid) {
fprintf(stderr, "read_bag\n");
	cur = cur->xmlChildrenNode;
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_bag: no storage info available");
		return NULL;
	}
	if(!find_element(&cur, "bagstorage")) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_bag: storage type mismatch");
		return NULL;
	}
	int num_marbles;
	get_int_attribute(cur, "num_marbles", num_marbles);
	BagTemplate * bat = new BagTemplate(myid); 
	if(bat == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:read_bag: Could not create mixed bag");
		return NULL;
	}
	cur = cur->xmlChildrenNode;
	while(cur != NULL) {
		if(name_match(cur, "marble")) {
			parse_marble(cur, bat); 
		}
		cur = cur->next;
	}
	return bat;
}

// parse a BagTemplate marble
void XMLReader::parse_marble(xmlNodePtr cur, BagTemplate * bat) { 
fprintf(stderr, "parse_marble\n"); 
	if(cur == NULL) {
		BB_log(BB_LOG_SEVERE, "XMLReader:parse_marble: null pointer");
		return;
	}
	// get basics
	time_t id = 0;
	bool playonce = false;
	t_TAPUINT likelihood = 0;
	double randomness = 1;
	long id_long = 0;
	xmlNodePtr next = cur->xmlChildrenNode; 
	while(next != NULL) {
		if(read_long_if(next, "id", id_long))
			id = (time_t)id_long;
		read_bool_if(next, "play_once", playonce); 
		read_tapuint_if(next, "likelihood", likelihood); 
		read_double_if(next, "randomness", randomness); 
		next = next->next;
	}
	// prepare to read template if appropriate
	UI_Template *uit = NULL, *dummy = NULL;
	Template *temp = NULL;
	Marble *mabel = NULL;
	// check whether template is already in library
	for(int k = 0; k < Library::instance()->size(); k++) {
		if(Library::instance()->templates[k]->core->id == id) { // jackpot
			uit = Library::instance()->templates[k];
			break;
		}
	}
	// if template does not exist in library
	if(uit == NULL) {
		// read template info from xml if available
		cur = cur->xmlChildrenNode; // update cur
		if(find_element(&cur, "template")) {
			temp = parse_template(cur); 
			if(temp != NULL) 
				uit = Library::instance()->add(temp); 
			else
				BB_log(BB_LOG_SEVERE, "XMLReader:parse_marble: could not read template"); 
		}
		else {
			BB_log(BB_LOG_WARNING, "XMLReader:parse_marble: could not find template information"); 
		}
	}
	// now if we have a template, place it in the bag in two steps
	if(uit != NULL) {
		// make dummy copy
		dummy = Library::instance()->add(uit->core); 
		uit->makedummy(dummy); 
		// place dummy in bag
		mabel = bat->insert(dummy);
		if(mabel) { 
			mabel->playonce = playonce;
			mabel->likelihood = likelihood;
			mabel->random = randomness;
			BB_log(BB_LOG_INFO, "XMLReader:parse_marble: added template %s to bag", uit->core->name.c_str());
		}
		else {
			BB_log(BB_LOG_WARNING, "XMLReader:parse_marble: could not add template %s to bag", dummy->core->name.c_str()); 
		}
	}
}


// general xml parsing functions
bool XMLReader::read_int_if(xmlNodePtr cur, const char * the_name, int &the_val) {
	if(name_match(cur, the_name)) {
		xmlChar * temp_xml = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); 
		the_val = atoi((const char *)temp_xml); 
		xmlFree(temp_xml);
		return true;
	}
	return false;
}

bool XMLReader::read_long_if(xmlNodePtr cur, const char * the_name, long &the_val) {
	if(name_match(cur, the_name)) {
		xmlChar * temp_xml = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); 
		the_val = atol((const char *)temp_xml); 
		xmlFree(temp_xml);
		return true;
	}
	return false;
}

bool XMLReader::read_double_if(xmlNodePtr cur, const char * the_name, double &the_val) {
	if(name_match(cur, the_name)) {
		xmlChar * temp_xml = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); 
		the_val = atof((const char *)temp_xml); 
		xmlFree(temp_xml);
		return true;
	}
	return false;
}

bool XMLReader::read_string_if(xmlNodePtr cur, const char * the_name, char * the_val) {
	if(name_match(cur, the_name)) {
		xmlChar * temp_xml = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		strcpy(the_val, (const char *)temp_xml); 
		xmlFree(temp_xml);
		return true;
	}
	return false;
}

bool XMLReader::read_taptime_if(xmlNodePtr cur, const char * the_name, t_TAPTIME &the_val) {
	return read_double_if(cur, the_name, the_val); 
}

bool XMLReader::read_tapuint_if(xmlNodePtr cur, const char * the_name, t_TAPUINT &the_val) {
	long a;
	bool ok = read_long_if(cur, the_name, a); 
	if(ok) the_val = (t_TAPUINT)a; 
	return ok;
}

bool XMLReader::read_tapbool_if(xmlNodePtr cur, const char * the_name, t_TAPBOOL &the_val) {
	return read_tapuint_if(cur, the_name, the_val);
}

bool XMLReader::read_float_if(xmlNodePtr cur, const char * the_name, float &the_val) {
	double a;
	bool ok = read_double_if(cur, the_name, a);
	if(ok) the_val = (float)a;
	return ok;
}

bool XMLReader::read_bool_if(xmlNodePtr cur, const char * the_name, bool &the_val) {
	int a;
	bool ok = read_int_if(cur, the_name, a);
	if(ok) the_val = a > 0;
	return ok;
}

void XMLReader::get_int_attribute(xmlNodePtr cur, const char * the_name, int &the_val) {
	try {
		xmlChar * temp_xml = xmlGetProp(cur, (const xmlChar *)the_name); 
		the_val = atoi((char *)temp_xml);
		xmlFree(temp_xml);
	} catch (...) {
		BB_log(BB_LOG_SEVERE, "XMLReader:get_int_attribute: error for attribute %s of element %s", 
			   the_name, (const char *)cur->name); 
	}
}

void XMLReader::get_long_attribute(xmlNodePtr cur, const char * the_name, long &the_val) {
	try {
		xmlChar * temp_xml = xmlGetProp(cur, (const xmlChar *)the_name); 
		the_val = atol((const char *)temp_xml);
		xmlFree(temp_xml);
	} catch (...) {
		BB_log(BB_LOG_SEVERE, "XMLReader:get_long_attribute: error for attribute %s of element %s", 
			   the_name, (const char *)cur->name); 
	}
}

void XMLReader::get_double_attribute(xmlNodePtr cur, const char * the_name, double &the_val) {
	try {
		xmlChar * temp_xml = xmlGetProp(cur, (const xmlChar *)the_name); 
		the_val = atof((const char *)temp_xml);
		xmlFree(temp_xml);
	} catch (...) {
		BB_log(BB_LOG_SEVERE, "XMLReader:get_double_attribute: error for attribute %s of element %s", 
			   the_name, (const char *)cur->name); 
	}
}

void XMLReader::get_taptime_attribute(xmlNodePtr cur, const char * the_name, t_TAPTIME &the_val) {
	get_double_attribute(cur, the_name, the_val); 
}

void XMLReader::get_tapuint_attribute(xmlNodePtr cur, const char * the_name, t_TAPUINT &the_val) {
	long a;
	get_long_attribute(cur, the_name, a); 
	the_val = (t_TAPUINT)a; 
}

void XMLReader::get_tapbool_attribute(xmlNodePtr cur, const char * the_name, t_TAPBOOL &the_val) {
	get_tapuint_attribute(cur, the_name, the_val);
}

void XMLReader::get_float_attribute(xmlNodePtr cur, const char * the_name, float &the_val) {
	double a;
	get_double_attribute(cur, the_name, a);
	the_val = (float)a;
}

void XMLReader::get_bool_attribute(xmlNodePtr cur, const char * the_name, bool &the_val) {
	int a;
	get_int_attribute(cur, the_name, a);
	the_val = (bool)a;
}

bool XMLReader::name_match(xmlNodePtr cur, const char * the_name) {
	return !xmlStrcmp(cur->name, (const xmlChar *)the_name);
}

bool XMLReader::find_element(xmlNodePtr *cur, const char * the_name) {
	bool matched = false;
	while(cur != NULL && *cur != NULL) {
		if(name_match(*cur, the_name)) {
			matched = true;
			break;
		} 
		*cur = (*cur)->next;
	}
	return matched;
}
#endif

// TAP READER

bool TapReader::open( char * filename )
{
    ifile.open( filename, ios::in );

/*	FILE * fd = fopen( filename, "r" );
	if( !fd ) {
		fprintf( stderr, "Cannot open file %s\n", filename );
	}
	else {
		fprintf( stderr, "Can open file %s, but not using ifile.open()?\n", filename );
	}

	fprintf( stderr, "%i %i %i\n", ifile.good(), ifile.bad(), ifile.fail() );*/
    return ifile.good();
}

void TapReader::close()
{
    ifile.close();
}

// templates
Template * TapReader::read_template()
{
    int type;
    time_t id; 
    std::string name, typestr; 
    double time_stretch, freq_warp, gain, pan, periodicity, density; 
    Template * tempt;

    // make sure file is open
    if( ifile.bad() )
    {
        msg_box( "internal errorrr", "cannot read from no file" );
        return 0;
    }
    
    ifile >> type >> id; 
    ifile >> name >> typestr >> time_stretch >> freq_warp >> gain >> pan >> periodicity >> density; 
    //if( id != 0 )id++; // id is now even & so unique

    // tempt gets created (new) in one of these functions
    switch( type )
    {
    case TT_DETERMINISTIC:
        tempt = read_det( id );
        break;
    case TT_RAW:
        tempt = read_raw( id );
        break;
    case TT_TRANSIENT:
        tempt = read_trans( id );
        break;
    case TT_FILE:
        tempt = read_file( id );
        break;
    case TT_RESIDUE:
        tempt = read_res( id );
        break;
    case TT_LOOP:
        tempt = read_loop( id );
        break;
    case TT_TIMELINE:
        tempt = read_tl( id );
        break;
    case TT_BAG:
        tempt = read_bag( id );
        break;
    default:
        tempt = NULL;
    }

    if( tempt != NULL )
    {
        tempt->name = name; 
        tempt->typestr = typestr;
        tempt->time_stretch = time_stretch;
        tempt->freq_warp = freq_warp;
        tempt->gain = gain;
        tempt->pan = pan;
        tempt->periodicity = periodicity;
        tempt->density = density;
        BB_log( BB_LOG_INFO, "Reading template %s of type %s", name.c_str(), typestr.c_str() );
    }
    else
    {
        BB_log( BB_LOG_WARNING, "Could not read template %s of type %s", name.c_str(), typestr.c_str() );
    }

    return tempt;
}

Deterministic * TapReader::read_det( time_t myid )
{
    std::vector<Track *> temp_tracks; 
    Deterministic * debt;
    
    if( read_tracks( temp_tracks ) )
        debt = new Deterministic( temp_tracks, myid );
    else
        debt = NULL;

    return debt;
}

Transient * TapReader::read_trans( time_t myid )
{
    Frame f;
    Transient * trance;
    
    if( read_frame( f ) )
        trance = new Transient( f, myid );
    else
        trance = NULL;
    
    return trance;
}

Raw * TapReader::read_raw( time_t myid )
{
    Frame f;
    Raw * war;
    
    if( read_frame( f ) )
        war = new Raw( f, myid );
    else
        war = NULL;
    
    return war;
}

File * TapReader::read_file( time_t myid )
{
    std::string path;
    char temp[1024];
    temp[0] = 0;
    ifile.getline( temp, 1 );
    ifile.getline( temp, 1024 );
    path = std::string( temp );

    File * phial;

    if( path.empty() )
    {
        phial = NULL;
        BB_log( BB_LOG_WARNING, "Could not read file with no name" );
    }
    else
    {
        phial = new File( path, myid );
        if( !phial->goodtogo )
            BB_log( BB_LOG_WARNING, "Could not read file %s", path.c_str() );
    }

    return phial;
}

Residue * TapReader::read_res( time_t myid )
{
    int total_levels, requested_levels, exists; 
    bool lefttree;

    ifile >> total_levels >> requested_levels >> lefttree; 
    
    // input file name
    ifile >> exists;
    if( exists ) {
		char filename[1024];
        ifile.getline( filename, 1024 ); 
		Residue * race = new Residue(filename, myid);
		Treesynth * tsth = race->ts;
		TreesynthIO * io = race->tsio;
		// left file name
		ifile >> exists; 
		if( exists )
			ifile.getline( io->leftfile, 1024 );
		// output file name
		ifile >> exists;
		if( exists )
			ifile.getline( io->ofilename, 1024 ); 
		// other io information
		ifile >> io->rm_mode >> io->write_to_buffer >> io->write_to_file;

		// read in new parameters
		ifile >> tsth->percentage >> tsth->kfactor >> tsth->stoplevel >> tsth->startlevel 
			  >> tsth->randflip >> tsth->ancfirst; 
		tsth->tree->resetLevels( total_levels );
		
		// done (no olar info stored in .tap)
		return race;
	}
	else {
		BB_log(BB_LOG_SEVERE, "TapReader:read_res: no filename available");
		return NULL;
	}
}

LoopTemplate * TapReader::read_loop( time_t myid )
{
    LoopTemplate * hole;
    double rand;
    ifile >> rand;
    
    Template * orig = read_template();
    if( orig != NULL )
        hole = new LoopTemplate( *orig, myid );
    
    hole->random = rand;

    return hole;
}


struct pseudoInstance
{
    time_t id;
    t_TAPTIME start_time;
    t_TAPTIME end_time;
    float y_offset;
};

Timeline * TapReader::read_tl( time_t myid )
{
    t_TAPTIME duration; 
    int loop, num_instances; 
    bool valid; 
    Template * temp, * poor; 
    UI_Template * uit, * dummy;

    ifile >> duration >> loop >> num_instances; 

    Timeline * teal = new Timeline( duration, myid ); 

    // instances
    pseudoInstance * inst = new pseudoInstance[num_instances];
    
    // read basic instance info
    for( int i = 0; i < num_instances; i++ )
    {
        ifile >> inst[i].id >> inst[i].start_time >> inst[i].end_time >> inst[i].y_offset;
        //inst[i].id += 1;
    }


    // read/match actual instances
    for( int j = 0; j < num_instances; j++ )
    {
        dummy = NULL;
        uit = NULL;

        // match: see if it is already in library
        for( int k = 0; k < Library::instance()->size(); k++ )
        {
            if( Library::instance()->templates[k]->core->id == inst[j].id ) // jackpot
            {   
                uit = Library::instance()->templates[k]; 
                break;
            }
        }

        // read in info if it's in .tap file; 
        ifile >> valid; 
        if( valid )
        {
            // already have a match; ignore this info
            if( uit != NULL )
            {
                poor = read_template();
                SAFE_DELETE( poor );
            }
            // no match found; use this info
            else
            {
                // create main template
                temp = read_template();
                uit = Library::instance()->add( temp );
            }
        }

        // if there is a template, place on timeline in two steps
        if( uit != NULL )
        {
            // make dummy copy
            Library::instance()->add( uit->core );
            dummy = Library::instance()->templates.back();
            uit->makedummy( dummy );

            // place dummy on timeline
            teal->place( dummy, 
                         (double) inst[j].start_time / teal->duration, 
                         inst[j].y_offset );
        }
    }

    SAFE_DELETE_ARRAY( inst );

    return teal;
}       

struct pseudoMarble
{
    time_t id; 
    bool playonce;
    t_TAPUINT likelihood;
    double random;
};

BagTemplate * TapReader::read_bag( time_t myid )
{
    int num_marbles; 
    bool valid; 
    Template * temp, * poor; 
    UI_Template * uit, * dummy;
    Marble * mabel;

    ifile >> num_marbles; 

    BagTemplate * bat = new BagTemplate( myid ); 

    // marbles
    pseudoMarble * mab = new pseudoMarble[num_marbles];
    
    // read basic marbles info
    for( int i = 0; i < num_marbles; i++ )
    {
        ifile >> mab[i].id >> mab[i].playonce >> mab[i].likelihood >> mab[i].random;
    }

    // read/match actual marbles
    for( int j = 0; j < num_marbles; j++ )
    {
        dummy = NULL;
        uit = NULL;

        // match: see if it is already in library
        for( int k = 0; k < Library::instance()->size(); k++ )
        {
            if( Library::instance()->templates[k]->core->id == mab[j].id )  // jackpot
            {   
                uit = Library::instance()->templates[k]; 
                break;
            }
        }

        // read in info if it's in .tap file; 
        ifile >> valid; 
        if( valid )
        {
            // already have a match; ignore this info
            if( uit != NULL )
            {
                poor = read_template();
                SAFE_DELETE( poor );
            }
            // no match found; use this info
            else
            {
                // create main template
                temp = read_template();
                uit = Library::instance()->add( temp );
            }
        }

        // if there is a template, place in bag in two steps
        if( uit != NULL )
        {
            // make dummy copy
            dummy = Library::instance()->add( uit->core );
            uit->makedummy( dummy );

            // place dummy in bag
            mabel = bat->insert( dummy );
            if( mabel )
            {
                mabel->playonce = mab[j].playonce;
                mabel->likelihood = mab[j].likelihood;
                mabel->random = mab[j].random;
            }
        }
    }

    SAFE_DELETE_ARRAY( mab );

    return bat;
}

// other things (parts of templates)
int TapReader::read_tracks( std::vector<Track *> &event )
{   
    Track * t;
    freqpolar temp_fp;
    std::string junk;
    int num_tracks, track_size;
    
    ifile >> junk >> num_tracks;

    for( int i = 0; i < num_tracks; i++ )
    {
        ifile >> junk >> track_size;
        t = new Track;
        ifile >> t->id >> t->state >> t->start >> t->end >> t->phase >> t->historian;

        for( int j = 0; j < track_size; j++ )
        {
            ifile >> temp_fp.freq >> temp_fp.p.mag >> temp_fp.p.phase >> temp_fp.time
                  >> temp_fp.bin >> temp_fp.isMatched; 
            t->history.push_back( temp_fp );
        }
        event.push_back( t );
    }
    
    return num_tracks;
}

int TapReader::read_frame( Frame &frame )
{
    std::string junk;
    t_TAPUINT wlen, wsize;

    ifile >> junk;
    ifile >> wlen >> wsize >> frame.time; // this time is now of type t_TAPTIME!!

    frame.alloc_waveform( wlen );
    frame.wsize = wsize;

    for( int i = 0; i < frame.wsize; i++ )
    {
        ifile >> frame.waveform[i]; 
    }
    
    return frame.wsize;
}

