//-----------------------------------------------------------------------------
// name: f1.ck
// desc: manipulating violence to make sound
//-----------------------------------------------------------------------------

// allocate taps template
TapsTemp fight;

//-----------------------------------------------------------------------------
// read templates from file
//-----------------------------------------------------------------------------
fun void load( TapsTemp temp, string name )
{
    // load it from library
    temp.readFromLibrary( name, 0 );
    // make sure it loaded
    if( !temp.ready() )
    {
        <<< "cannot load:", name >>>;
        me.exit();
    }
}

// load
load( fight, "fight1" );

// synch
.5::second => dur T;
T - ( now % T ) => now;

// infinite time-loop
while( true )
{
    std.rand2f( .25, .75 ) => fight.pan;
    std.rand2f( .5, 1.5 ) => fight.gain;
    fight.play();
    T => now;
}


