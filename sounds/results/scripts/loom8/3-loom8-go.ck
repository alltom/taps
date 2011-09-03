//-----------------------------------------------------------------------------
// name: loom8-live.ck
// desc: the piece
//       loom8.ck should be executed before running this
//-----------------------------------------------------------------------------

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

fun void section_live()
{
	// children
	for( int i; i < 9; i++ )
    {
	    // jack up the timestretch
		Loom.stretchTo( children[i+1], Std.rand2f( 25, 80 ) );
    }
	
	// freq
	children[1].freqWarp( 1.0 );
	children[2].freqWarp( 1.0 );
	children[3].freqWarp( 1.0 );
	children[4].freqWarp( 1.0 );
	children[5].freqWarp( 1.0 );
	children[6].freqWarp( 1.0 );
	children[7].freqWarp( 1.0 );
	children[8].freqWarp( 1.0 );
	children[9].freqWarp( 1.0 );
}

section_live();
