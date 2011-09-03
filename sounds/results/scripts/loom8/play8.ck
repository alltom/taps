// get number of channels
dac.channels() => int channels;
// check it
if( channels != 8 )
{
    <<< "not enough channels!", "" >>>;
    me.exit();
}

// allocate sound bufs
SndBuf chan[channels];

// filename
"a.wav" => string filename;

// read different channels
for( int i; i < channels; i++ )
{
    // no
    0 => chan[i].interp;
    // set chunks
    256 => chan[i].chunks;
    // read
    filename => chan[i].read;
    // check
    if( i == 0 && chan[i].channels() != channels )
    {
        <<< "file is not", channels, "channels!" >>>;
        me.exit();
    }

    // set channel
    chan[i].channel(i);
    // connect to
    chan[i] => dac.chan(i);
}

// play
chan[0].length() => now;

// print out
<<< "done!", "" >>>;
