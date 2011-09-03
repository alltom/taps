//-----------------------------------------------------------------------------
// name: fight_load.ck
// desc: loading things into synthesis face
//-----------------------------------------------------------------------------

fun void load( string name )
{
    if( !TapsSynth.load( name ) )
    {
        <<< "load.ck: cannot load", name >>>;
        me.exit();
    }
}

load( "fight1" );
load( "fight2" );
load( "fight3" );
load( "fight4" );
load( "lutinebell" ); TapsSynth.copy( "lutinebell", 0, 3 );
load( "italyhorn" ); TapsSynth.copy( "italyhorn", 0, 3 );
load( "glassbreak" );
load( "baby" );
load( "chirp1" );
load( "chirp2" );
load( "chirp1loop" );
