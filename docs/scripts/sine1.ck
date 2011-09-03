sinosc s => dac;
.25 => s.gain;

0.0 => float t;

while( true )
{
    (100.0 + std.abs(300.0 * math.sin(t))) => s.sfreq;
    t + .01 => t;
    5::ms => now;
}
