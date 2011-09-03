//-----------------------------------------------------------------------------
// name: f2.ck
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
load( fight, "fight2" );

// synch
.5::second => dur T;
T - ( now % T ) => now;

1 => float factor;
[ 4, 8, 2, 8, 2, 2, 4, 8, 6, 2, 2, 8, 2 ] @=> int wait[];
int i;

// infinite time-loop
while( true )
{
    std.rand2f( .25, .75 ) => fight.pan;
    std.rand2f( .5, 1.2 ) => fight.gain;
    fight.play();
    factor*wait[i]::T => now;
    i++; i % wait.cap() => i;
}

