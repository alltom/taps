#include "ui_data.h"

//-----------------------------------------------------------------------------
// name: struct val_u
// desc: to store miscellaneous data types in one place
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: val_u
// desc: constructor to create invalid val_u, like for std::map to use
//-----------------------------------------------------------------------------
val_u::val_u() 
{
	datatype = V_INVALID;
	timestamp = 0;
	Time= 0; Float = 0; Uint = 0; t = 0; // should cover all bits, I think
}

//-----------------------------------------------------------------------------
// name: val_u
// desc: constructor
//-----------------------------------------------------------------------------
val_u::val_u( t_TAPUINT type, t_TAPTIME stamp ) 
{
	datatype = type;
	timestamp = stamp;
	Time= 0; Float = 0; Uint = 0; t = 0; // should cover all bits, I think
}

//-----------------------------------------------------------------------------
// name: get_timestamp
// desc: accessor
//-----------------------------------------------------------------------------
t_TAPTIME val_u::get_timestamp()
{
	return timestamp;
}

//-----------------------------------------------------------------------------
// name: get_datatype
// desc: accessor
//-----------------------------------------------------------------------------
t_TAPUINT val_u::get_datatype()
{
	return datatype;
}

//-----------------------------------------------------------------------------
// name: debug
// desc: print 
//-----------------------------------------------------------------------------
void val_u::debug()
{
	fprintf( stderr, "struct val_u: time: %f | ", this->timestamp );
	switch( this->datatype ) {
	case V_INVALID: 
		fprintf( stderr, "type: invalid\n" );
		break;
	case V_S:
		fprintf( stderr, "type: std::string | value: %s\n", this->s.c_str() );
		break;
	case V_TIME:
		fprintf( stderr, "type: t_TAPTIME | value: %f\n", this->Time );
		break;
	case V_FLOAT:
		fprintf( stderr, "type: t_TAPFLOAT | value: %f\n", this->Float );
		break;
	case V_INT:
		fprintf( stderr, "type: t_TAPINT | value: %d\n", this->Int );
		break;
	case V_UINT:
		fprintf( stderr, "type: t_TAPUINT | value: %u\n", this->Uint );
		break;
	case V_BOOL:
		fprintf( stderr, "type: t_TAPBOOL | value: %b\n", this->Bool );
		break;
	case V_C:
		fprintf( stderr, "type: char | value: %c\n", this->c );
		break;
	case V_T:
		fprintf( stderr, "type: time_t | value: %u\n", this->t );
		break;
	default:
		fprintf( stderr, "type: undefined!\n" );
		break;
	}
}


//-----------------------------------------------------------------------------
// name: class TimeData
// desc: to store miscellaneous data types within one type, using val_u
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: TimeData
// desc: constructor
//-----------------------------------------------------------------------------
TimeData::TimeData( t_TAPUINT type )
{
	datatype = type;
}

//-----------------------------------------------------------------------------
// name: get_datatype
// desc: accessor
//-----------------------------------------------------------------------------
t_TAPUINT TimeData::get_datatype()
{
	return datatype;
}

//-----------------------------------------------------------------------------
// name: set_val
// desc: modify value (should it copy or use the given address?)
//-----------------------------------------------------------------------------
void TimeData::set_val( t_TAPTIME when, void * data )
{
	// set value's appropriate field
	val_u value( datatype, when );
	switch( datatype ) {
	case V_S:
		value.s = *((std::string *)data); break;
	case V_TIME:
		value.Time = *((t_TAPTIME *)data); break;
	case V_FLOAT:
		value.Float = *((t_TAPFLOAT *)data); break;
	case V_INT:
		value.Int = *((t_TAPINT *)data); break;
	case V_UINT:
		value.Uint = *((t_TAPUINT *)data); break;
	case V_BOOL:
		value.Bool = *((t_TAPBOOL *)data); break;
	case V_C:
		value.c = *((t_TAPBOOL *)data); break;
	case V_T:
		value.t = *((time_t *)data); break;
	default:
		BB_log( BB_LOG_SYSTEM, "Data type %u not recognized, no value set", datatype );
	}

	// insert in map
	timedata[when] = value;
}


//-----------------------------------------------------------------------------
// name: get_val
// desc: retrieve value
//-----------------------------------------------------------------------------
val_u TimeData::get_val( t_TAPTIME when, val_u (*interp_ptr)( val_u, val_u, t_TAPTIME ) )
{
	// anything?
	if( timedata.empty() )
		return val_u();

	// find first element whose key is not less than "when"
	std::map<t_TAPTIME, val_u>::iterator iter = timedata.lower_bound( when );
	
	// if exact, done
	if( iter->first == when )
		return iter->second;
	
	// otherwise, only one element, or first element?
	if( timedata.size() == 1 || iter == timedata.begin() )
	{
		if( interp_ptr != NULL )
			return interp_ptr( iter->second, iter->second, when );
		else
			return val_u();
	}

	// even last element is too small 
	if( iter == timedata.end() )
	{
		--iter;
		if( interp_ptr != NULL )
			return interp_ptr( iter->second, iter->second, when );
		else
			return val_u();
	}

	// otherwise, interpolate
	if( interp_ptr != NULL )
	{
		val_u b = iter->second;
		val_u a = (--iter)->second;
		return interp_ptr( a, b, when );
	}
	else
		return val_u();
}

//-----------------------------------------------------------------------------
// name: get_vals
// desc: retrieve all values (entire map)
//-----------------------------------------------------------------------------
std::map<t_TAPTIME, val_u> TimeData::get_vals()
{
	return timedata;
}

//-----------------------------------------------------------------------------
// name: get_index
// desc: get indexes of entries surrounding given time point
//		 (prev = next if that point exists)
//-----------------------------------------------------------------------------
void get_index( t_TAPTIME when, t_TAPINT &prev, t_TAPINT &next )
{
	// binary search
	t_TAPINT start = 0; 
	t_TAPINT end = timedata.length();
	t_TAPINT mid = 0;
	while( start < end )
	{
		mid = (start + end) / 2;
		t_TAPTIME midtime = timedata[mid].get_timestamp();
		if( midtime > when )
			end = mid;
		else if( midtime < when )
			start = mid + 1;
		else
			break;
	}

	// end points
	t_TAPTIME midtime = timedata[mid].get_timestamp();
	if( mid < 0 ) {
		prev = -1; 
		next = 0;
	}
	else if( mid >= timedata.length() ) {
		prev = timedata.length() - 1;
		next = prev + 1;
	}
	else if( midtime == when )
		prev = next = mid;
	else if( midtime > when ) {
		next = mid;
		prev = mid - 1;
		if( prev >= 0 && timedata[prev].get_timestamp() >= when )
			BB_log( BB_LOG_WARNING, "get_index() error" );
	}
	else {
		BB_log( BB_LOG_WARNING, "get_index() error" );
		BB_log( BB_LOG_WARNING, "time[%d] = %f; want = %f", mid, midtime, when );
		BB_log( BB_LOG_WARNING, "time[%d] = %f", mid - 1, mid - 1 >= 0 ? timedata[mid-1].get_timestamp() : 0 );
		BB_log( BB_LOG_WARNING, "time[%d] = %f", mid + 1, mid + 1 < timedata.length() ? timedata[mid+1].get_timestamp(), 0 );
	}
}

//-----------------------------------------------------------------------------
// name: debug
// desc: print
//-----------------------------------------------------------------------------
void TimeData::debug()
{
	std::string typestr;
	switch( this->datatype ) {
	case V_INVALID: 
		typestr = "invalid"; break;
	case V_S:
		typestr = "std::string"; break;
	case V_TIME:
		typestr = "t_TAPTIME"; break;
	case V_FLOAT:
		typestr = "t_TAPFLOAT"; break;
	case V_INT:
		typestr = "t_TAPINT"; break;
	case V_UINT:
		typestr = "t_TAPUINT"; break;
	case V_BOOL:
		typestr = "t_TAPBOOL"; break;
	case V_C:
		typestr = "char"; break;
	case V_T:
		typestr = "time_t"; break;
	default:
		typestr = "undefined"; break;
	}

	fprintf( stderr, "class TimeData: type: %s | data:\n", typestr.c_str() );
	std::map<t_TAPTIME, val_u>::iterator iter;
	for( iter = timedata.begin(); iter != timedata.end(); iter++ )
	{
		fprintf( stderr, "%f -- ", iter->first );
		iter->second.debug();
	}
}


//-----------------------------------------------------------------------------
// name: class Snapshot
// desc: contains parameter info
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: Snapshot
// desc: constructor
//-----------------------------------------------------------------------------	
Snapshot::Snapshot( Template2 * parent ) 
{
	this->parent = parent;
}

//-----------------------------------------------------------------------------
// name: ~Snapshot
// desc: destructor
//-----------------------------------------------------------------------------	
Snapshot::~Snapshot() {}

//-----------------------------------------------------------------------------
// name: add_param
// desc: add a parameter, assume param doesn't already exist
//-----------------------------------------------------------------------------	
void Snapshot::add_param( std::string name, t_TAPUINT type )
{
	params.insert( make_pair<std::string, TimeData>( name, TimeData(type) ) );
	// how does this relate to params[name] = TimeData(type)?
}


//-----------------------------------------------------------------------------
// name: retrieve_param_data
// desc: retrieve data identified by name
//-----------------------------------------------------------------------------	
TimeData * Snapshot::retrieve_param_data( std::string name )
{
	std::map<std::string, TimeData>::iterator iter = params.find( name );
	if( iter != params.end() )
		return &(iter->second);
	else
		BB_log( BB_LOG_WARNING, "parameter '%s' not found", name.c_str() );
	return NULL;
}

//-----------------------------------------------------------------------------
// name: set_param
// desc: set param's value for given name and timestamp
//-----------------------------------------------------------------------------	
void Snapshot::set_param( std::string name, t_TAPTIME when, void * value )
{
	TimeData * param = retrieve_param_data( name );
	if( param )
		param->set_val( when, value );
}

//-----------------------------------------------------------------------------
// name: get_param
// desc: get some parameter value, given optional function pointer
//		 for interpolation
//-----------------------------------------------------------------------------	
val_u Snapshot::get_param( std::string name, t_TAPTIME when, val_u (*interp_ptr)( val_u, val_u, t_TAPTIME ) )
{
	val_u v;
	TimeData * param = retrieve_param_data( name );
	if( param )
		v = param->get_val( when, interp_ptr );
	return v;
}

//-----------------------------------------------------------------------------
// name: get_param_series
// desc: get entire parameter series (or a copy(?) of the actual TimeData object)
//-----------------------------------------------------------------------------	
TimeData Snapshot::get_param_series( std::string name )
{
	TimeData * tptr = retrieve_param_data( name );
	if( tptr != NULL )
		return *tptr;
	else 
		return TimeData( V_INVALID );
}

//-----------------------------------------------------------------------------
// name: debug
// desc: print
//-----------------------------------------------------------------------------	
void Snapshot::debug()
{
	fprintf( stderr, "class Snapshot: parent: 0x%x; params:\n", parent );
	std::map<std::string, TimeData>::iterator iter;
	for( iter = params.begin(); iter != params.end(); iter++ )
	{
		fprintf( stderr, "name: %s | timedata:\n", iter->first.c_str() );
		iter->second.debug();
	}
}


//-----------------------------------------------------------------------------
// TESTING!!!11!!111!!!!!1111!!!
//-----------------------------------------------------------------------------	
val_u linear_interpolation( val_u a, val_u b, t_TAPTIME when )
{
	assert( a.get_datatype() == b.get_datatype() );
	t_TAPUINT type = a.get_datatype();
	assert( type == V_UINT || type == V_INT || type == V_FLOAT || type == V_TIME );

	// decode
	double aval, bval;
	switch( type ) {
	case V_UINT:
		aval = a.Uint; 
		bval = b.Uint;
		break;
	case V_INT:
		aval = a.Int;
		bval = b.Int;
		break;
	case V_FLOAT:
		aval = a.Float;
		bval = b.Float;
		break;
	case V_TIME:
		aval = a.Time;
		bval = b.Time;
		break;
	}

	// compute
	double cval;
	double denom = b.get_timestamp() - a.get_timestamp();
	if( denom < 0.1e-6 ) 
		cval = aval;
	else {
		double w = (when - a.get_timestamp()) / denom;
		cval = aval + w * (bval - aval);
	}

	// encode
	val_u c( type, when );
	switch( type ) {
	case V_UINT: 
		c.Uint = (t_TAPUINT)cval;
		break;
	case V_INT:
		c.Int = (t_TAPINT)cval;
		break;
	case V_FLOAT:
		c.Float = (t_TAPFLOAT)cval;
		break;
	case V_TIME:
		c.Time = (t_TAPTIME)cval;
		break;
	}

	return c;
}


//-----------------------------------------------------------------------------
// name: class Template2
// desc: perhaps the new Template
//-----------------------------------------------------------------------------
Template2::Template2() {}

Template2::~Template2() {}


int main( void ) 
{
fprintf( stderr, "Point 1\n" );
	Template2 bs1, bs2;
	std::vector<Snapshot> shots;
	Snapshot s( &bs1 );
	s.add_param( "frequency", V_FLOAT );
	s.add_param( "pianicity", V_UINT );

	t_TAPFLOAT float_data;
	t_TAPUINT uint_data;
fprintf( stderr, "Point 2\n" );
	float_data = 2.5; s.set_param( "frequency", 1234, &float_data );
	float_data = 10.0; s.set_param( "frequency", 2234, &float_data );
	float_data = 3.8; s.set_param( "frequency", 2000, &float_data );
	shots.push_back( s );
	Snapshot * s2 = &(shots[0]);
	uint_data = 2; s2->set_param( "pianicity", 1234, &uint_data );
	uint_data = 3; s2->set_param( "pianicity", 2234, &uint_data );
	uint_data = 4; s2->set_param( "pianicity", 3234, &uint_data );
	s2->debug();
fprintf( stderr, "Point 3\n" );	
	val_u a = s2->get_param( "pianicity", 3234, &linear_interpolation ); a.debug();
	a = s2->get_param( "pianicity", 3000, &linear_interpolation ); a.debug();
	a = s2->get_param( "pianicity", 3000, NULL ); a.debug();
	a = s2->get_param( "pianicity", 4000, &linear_interpolation ); a.debug();
	a = s2->get_param( "pianicity", 4000, NULL ); a.debug();
fprintf( stderr, "Point 4\n" );
	val_u b = s2->get_param( "frequency", 1234, NULL ); b.debug();
	b = s2->get_param( "frequency", 2234, NULL ); b.debug();
	b = s2->get_param( "frequency", 2000, NULL ); b.debug();
	b = s2->get_param( "frequency", 2100, &linear_interpolation ); b.debug();
	b = s2->get_param( "frequency", 2100, NULL ); b.debug(); 
fprintf( stderr, "Point 5\n" );
	TimeData t = s2->get_param_series( "pianicity" );
	t.debug();
	val_u c = t.get_val( 3234 ); c.debug();
	uint_data = 5; t.set_val( 3234, &uint_data );
	c = t.get_val( 3234 ); c.debug();
	t = s2->get_param_series( "pianicity" );
	c = t.get_val( 3234 ); c.debug();
	t.debug();
fprintf( stderr, "Point 6\n" );
	std::map<t_TAPTIME, val_u> vals = t.get_vals();
	std::map<t_TAPTIME, val_u>::iterator iter;
	for( iter = vals.begin(); iter != vals.end(); iter++ ) {
		fprintf( stderr, "%f ", iter->first );
		iter->second.debug();
	}
fprintf( stderr, "Point 7\n" );
	return 0;
}