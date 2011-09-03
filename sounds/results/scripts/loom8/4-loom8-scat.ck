//-----------------------------------------------------------------------------
// name: loom8-scat.ck
// desc: the piece
//       loom8.ck should be executed before running this
//-----------------------------------------------------------------------------

// number of channels
8 => int channels;

// scale ratios
float s[12];
for( int i; i < s.cap(); i++ )
    std.mtof( i + 60 ) / std.mtof( 60 ) => s[i];

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

fun void no()
{
    <<< "******************start********************", "" >>>;

    // children
	float f;
	dur T;
	for( int i; i < 9; i++ )
    {
	    // randomize
		spork ~ Loom.freqTo( children[i+1], Std.rand2f( .1, 1 ) => f, (10 + Std.rand2f(5, 15))::second => T );
		// print
		<<< "freq:", i, f >>>;
    }
	
	T => now;
	
	<<< "***************** end **********************", "" >>>;
}

no();
