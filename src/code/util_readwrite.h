#ifndef __UTIL_READWRITE_H__
#define __UTIL_READWRITE_H__

//#include <stdlib.h>
//#include <memory.h>
//#include <string>
#include "ui_library.h"
#include "taps_def.h"
#include "taps_birdbrain.h"
#ifdef __TAPS_XML_ENABLE__
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#endif

#ifdef __TAPS_XML_ENABLE__
// xml document builder
struct XMLBuilder {
	XMLBuilder(); 
	~XMLBuilder();
	bool start_document(const char * rootstring);
	bool end_document();
	bool start_element(const char * name);
	bool end_element();
	bool format_element(const char * name, const char * format, ...);
	bool format_attribute(const char * name, const char * format, ...);
	bool add_element(const char * name, const char * content);
	bool add_attribute(const char * name, const char * value);
	bool add_branch(xmlNodePtr branch);
	xmlDocPtr get_doc(); 
	xmlDocPtr get_doc_copy();
	static xmlDocPtr copy(xmlDocPtr orig);
protected:
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr cur;
	char the_buffer[1024];
	int the_bufsize;
}; 
#endif

// Writing classes
struct TemplateWriter
{
	virtual bool open(char *filename);
	virtual bool close();
	virtual int write_template(const Template * tempt, void * analinfo = NULL); 
	
protected: 
	TemplateWriter * tw;
};

#ifdef __TAPS_XML_ENABLE__
struct XMLWriter : public TemplateWriter
{
	virtual bool open(char * fileName);
	virtual bool close();
	virtual int write_template(const Template * tempt, void * analinfo = NULL); 
	
	// special templates
	int store_det(const Deterministic * debt);
	int store_trans(const Transient * trance); 
	int store_raw(const Raw * war); 
	int store_res(const Residue * race); 
	int store_loop(const LoopTemplate * hole); 
	int store_tl(const Timeline * teal);
	int store_bag(const BagTemplate * bat); 
	
	// other template stuff
	int store_template(const Template * tempt, void * analinfo); 
	int store_frame(const Frame &frame);
	
protected:
	XMLBuilder builder;
	std::string filename;
};
#endif

struct TapWriter : public TemplateWriter
{
    std::ofstream ofile;

    virtual bool open(char * filename);
    virtual bool close();
    virtual int write_template(const Template * tempt, void * analinfo = NULL);

    // templates
    int write_det( const Deterministic * debt );
    int write_trans( const Transient * trance );
    int write_raw( const Raw * war );
    int write_file( const File * phial );
    int write_res( const Residue * race );
    int write_loop( const LoopTemplate * hole );
    int write_tl( const Timeline * teal );
    int write_bag( const BagTemplate * bat ); 

    // other things (parts of templates)
    int write_tracks( const std::vector<Track *> &event );
    int write_frame( const Frame &frame );
};


// Reading classes
struct TemplateReader
{
	virtual bool open(char * filename);
	virtual void close();
	virtual Template * read_template();
	
protected:
	TemplateReader * tr;
};

#ifdef __TAPS_XML_ENABLE__
struct XMLReader : public TemplateReader
{
	virtual bool open(char * filename);
	virtual void close();
	virtual Template * read_template();
	
	// special templates
	Deterministic * read_det(xmlNodePtr cur, time_t myid = 0); 
	Transient * read_trans(xmlNodePtr cur, time_t myid = 0);
	Raw * read_raw(xmlNodePtr cur, time_t myid = 0);
	Residue * read_res(xmlNodePtr cur, time_t myid = 0); 
	LoopTemplate * read_loop(xmlNodePtr cur, time_t myid = 0);
	Timeline * read_tl(xmlNodePtr cur, time_t myid = 0);
	BagTemplate * read_bag(xmlNodePtr cur, time_t myid = 0); 
	
	// other template stuff
	Template * parse_template(xmlNodePtr curroot); 
	void parse_synthesis_info(xmlNodePtr cur, Template * tempt); 
	int read_tracks(xmlNodePtr cur, std::vector<Track *> &event);
	Track * parse_track(xmlNodePtr cur);
	bool parse_history(xmlNodePtr cur, freqpolar &temp_fp);
	int read_frame(xmlNodePtr cur, Frame &frame);
	bool read_frame_samples(xmlNodePtr cur, Frame &frame); 
	bool read_res_name(xmlNodePtr cur, char filename[]); 
	void parse_instance(xmlNodePtr cur, Timeline * teal); 
	void parse_marble(xmlNodePtr cur, BagTemplate * bat); 

	// general xml parsing functions
	bool read_int_if(xmlNodePtr cur, const char *the_name, int &the_val);
	bool read_long_if(xmlNodePtr cur, const char *the_name, long &the_val);
	bool read_double_if(xmlNodePtr cur, const char *the_name, double &the_val);
	bool read_string_if(xmlNodePtr cur, const char *the_name, char * the_val); 
	bool read_taptime_if(xmlNodePtr cur, const char *the_name, t_TAPTIME &the_val);
	bool read_tapuint_if(xmlNodePtr cur, const char *the_name, t_TAPUINT &the_val);
	bool read_tapbool_if(xmlNodePtr cur, const char *the_name, t_TAPBOOL &the_val);
	bool read_float_if(xmlNodePtr cur, const char *the_name, float &the_val);
	bool read_bool_if(xmlNodePtr cur, const char *the_name, bool &the_val);
	void get_int_attribute(xmlNodePtr cur, const char *the_name, int &the_val);
	void get_long_attribute(xmlNodePtr cur, const char *the_name, long &the_val);
	void get_double_attribute(xmlNodePtr cur, const char *the_name, double &the_val);
	void get_taptime_attribute(xmlNodePtr cur, const char *the_name, t_TAPTIME &the_val);
	void get_tapuint_attribute(xmlNodePtr cur, const char *the_name, t_TAPUINT &the_val);
	void get_tapbool_attribute(xmlNodePtr cur, const char *the_name, t_TAPBOOL &the_val);
	void get_float_attribute(xmlNodePtr cur, const char *the_name, float &the_val);
	void get_bool_attribute(xmlNodePtr cur, const char *the_name, bool &the_val);
	bool name_match(xmlNodePtr cur, const char *the_name); 
	bool find_element(xmlNodePtr *cur, const char *the_name); 

protected:
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNsPtr ns;
};
#endif

struct TapReader : TemplateReader
{
    // internal
protected:
    std::ifstream ifile;

public:
    virtual bool open( char * filename );
    virtual void close();
	virtual Template * read_template();
    
    // templates
    Deterministic * read_det( time_t myid = 0 );
    Transient * read_trans( time_t myid = 0 );
    Raw * read_raw( time_t myid = 0 );
    File * read_file( time_t myid = 0 );
    Residue * read_res( time_t myid = 0 );
    LoopTemplate * read_loop( time_t myid = 0 );
    Timeline * read_tl( time_t myid = 0 );
    BagTemplate * read_bag( time_t myid = 0 ); 

    // other things (parts of templates)
    int read_tracks( std::vector<Track *> & event );
    int read_frame( Frame & frame );
};


#endif

