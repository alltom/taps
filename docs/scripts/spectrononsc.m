% SPECTRONONSC('orig') draws specgram of 'orig.wav' 
% SPECTRONONSC('orig','file1','file2',...) draws specgram of 'orig.wav' and
%   for each additional 'fileN.wav' with the same scaling as for 'orig.wav'
% Can be modified to save the specgtrograms by uncommenting some lines.
%
% Sometimes a simpler thing to try is get the specgram data, and make an 
% image of it without adding eps (which causes problems when there is
% silence; see "help specgram" for the imagesc arguments to use)

function spectrononsc(orig,varargin)

wav = strcat(orig, '.wav');

w = wavread(wav);
specgram(w(:,1), 2048, 44100);
cm = get(gcf,'Colormap');

[d,m,a,s] = specgramnonsc('give',0,0,0,0,cm,w(:,1),2048,44100);

for i = 1:nargin,
    if i == 1,
        fname = orig;
    else
        fname = varargin{i-1};
    end
    
    wav = strcat(fname, '.wav');
    w = wavread(wav);
    
    figure(i);
    specgramnonsc('take',d,m,a,s,cm,w(:,1),2048,44100);
    xlabel('Time (sec)');
    ylabel('Frequency (Hz)');
    set(get(gcf, 'CurrentAxes'), 'YTickLabel', ...
        [0 5000 10000 15000 20000]);

    pdf = strcat(fname, '.pdf');
    fig = strcat(fname, '.fig');
    jpg = strcat(fname, '.jpg');

%    saveas(gcf, fig);
    saveas(gcf, pdf);
%    saveas(gcf, jpg);
end