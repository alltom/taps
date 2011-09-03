% By Benedict Brown

function spectro(fname)

wav = strcat(fname, '.wav');
pdf = strcat(fname, '.pdf');
fig = strcat(fname, '.fig');
jpg = strcat(fname, '.jpg');

w = wavread(wav);
specgram(w(:,1), 2048, 44100);
xlabel('Time (sec)');
ylabel('Frequency (Hz)');
set(get(gcf, 'CurrentAxes'), 'YTickLabel', ...
    [0 5000 10000 15000 20000]);


%saveas(gcf, fig);
saveas(gcf, pdf);
%saveas(gcf, jpg);