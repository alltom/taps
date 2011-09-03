//-----------------------------------------------------------------------------
// name: loom8-start.ck
// desc: the piece
//       loom8.ck should be executed before running this
//-----------------------------------------------------------------------------

// number of channels
8 => int channels;

// initial settings on all buses, volume, reverb, pan
Loom.allSet( 1.0, 0.0, 0.0 );

// horn
TapsTemp horn;
// read from library
Loom.readFromLibrary( horn, "italyhorn" );

// bell
TapsTemp bells[8];
// copy
TapsSynth.copy( "lutinebell1", 0, bells.cap()-1 );
// read from library
Loom.readFromLibrary( bells, "lutinebell1" );
// set bus
for( int i; i < bells.cap(); i++ )
{
    bells[i].bus( i );
}

// squawk
TapsTemp squawk;
// read from library
Loom.readFromLibrary( squawk, "squawk" );

// children
TapsTemp children[10];
// read from Library
Loom.readFromLibrary( children[1], "child1" );
Loom.readFromLibrary( children[2], "child2" );
Loom.readFromLibrary( children[3], "child3" );
Loom.readFromLibrary( children[4], "child4" );
Loom.readFromLibrary( children[5], "child5" );
Loom.readFromLibrary( children[6], "child6" );
Loom.readFromLibrary( children[7], "child7" );
Loom.readFromLibrary( children[8], "child8" );
Loom.readFromLibrary( children[9], "child9" );

// quack4
TapsTemp quack4;
Loom.readFromLibrary( quack4, "quack4" );

for( 1 => int i; i < 9; i++ )
{
    children[i].bus( i - 1 );
}
children[9].bus( 1 );


// start timer
spork ~ Loom.timer( now );

fun void section_live()
{
    // bring up reverb
	for( int i; i < 8; i++ )
        spork ~ Loom.busReverbTo( i, .1, 1::second );

    // set squawk
	Loom.freqTo( squawk, 1.122 );
	Loom.stretchTo( squawk, 8.414 );
	Loom.gainTo( squawk, 1.5 );
	squawk.bus( 2 );

	// set quack4
	quack4.bus( 3 );
	
	1::second => now;
}

section_live();
