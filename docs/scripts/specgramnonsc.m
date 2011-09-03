function [div,mult,add,sub,yo,fo,to] = specgramnonsc(mode,div,mult,add,sub,cm,varargin)
%SPECGRAMNONSC Calculate spectrogram from signal.
%   [div,mult,add,sub,yo,fo,to] = SPECGRAMNONSC(mode,div,mult,add,sub,cm,varargin)
%   works like SPECGRAM but with some extra args in and out
%   MODE is either 'give' or 'take'
%   DIV, MULT, ADD, SUB are scaling information for the image
%   CM is the colormap, ideally the one used by SPECGRAM when called on the
%   given signal
%   VARARGIN - the usual SPECGRAM input
%   if MODE is 'give', it ignores the div,mult,add,sub input arguments and
%   generates its own, returned as output
%   if MODE is 'take', it takes the given arguments
%   good for specgrams of slightly modified versions of a signal so that
%   they have the same color scaling...
%   See also SPECGRAM.
%   Author(s): Me! :)
%   Copyright none
%   $Date: 2005/11/11 17:58:53 $

error(nargchk(7,11,nargin))
[msg,x,nfft,Fs,window,noverlap]=specgramchk(varargin);
error(msg)
    
nx = length(x);
nwind = length(window);
if nx < nwind    % zero-pad x if it has length less than the window length
    x(nwind)=0;  nx=nwind;
end
x = x(:); % make a column vector for ease later
window = window(:); % be consistent with data set

ncol = fix((nx-noverlap)/(nwind-noverlap));
colindex = 1 + (0:(ncol-1))*(nwind-noverlap);
rowindex = (1:nwind)';
if length(x)<(nwind+colindex(ncol)-1)
    x(nwind+colindex(ncol)-1) = 0;   % zero-pad x
end

if length(nfft)>1
    df = diff(nfft);
    evenly_spaced = all(abs(df-df(1))/Fs<1e-12);  % evenly spaced flag (boolean)
    use_chirp = evenly_spaced & (length(nfft)>20);
else
    evenly_spaced = 1;
    use_chirp = 0;
end

if (length(nfft)==1) | use_chirp
    y = zeros(nwind,ncol);

    % put x into columns of y with the proper offset
    % should be able to do this with fancy indexing!
    y(:) = x(rowindex(:,ones(1,ncol))+colindex(ones(nwind,1),:)-1);

    % Apply the window to the array of offset signal segments.
    y = window(:,ones(1,ncol)).*y;

    if ~use_chirp     % USE FFT
        % now fft y which does the columns
        y = fft(y,nfft);
        if ~any(any(imag(x)))    % x purely real
            if rem(nfft,2),    % nfft odd
                select = [1:(nfft+1)/2];
            else
                select = [1:nfft/2+1];
            end
            y = y(select,:);
        else
            select = 1:nfft;
        end
        f = (select - 1)'*Fs/nfft;
    else % USE CHIRP Z TRANSFORM
        f = nfft(:);
        f1 = f(1);
        f2 = f(end);
        m = length(f);
        w = exp(-j*2*pi*(f2-f1)/(m*Fs));
        a = exp(j*2*pi*f1/Fs);
        y = czt(y,m,w,a);
    end
else  % evaluate DFT on given set of frequencies
    f = nfft(:);
    q = nwind - noverlap;
    extras = floor(nwind/q);
    x = [zeros(q-rem(nwind,q)+1,1); x];
    % create windowed DTFT matrix (filter bank)
    D = window(:,ones(1,length(f))).*exp((-j*2*pi/Fs*((nwind-1):-1:0)).'*f'); 
    y = upfirdn(x,D,1,q).';
    y(:,[1:extras+1 end-extras+1:end]) = []; 
end

t = (colindex-1)'/Fs;

% quick scaling
pic = 20*log10(abs(y)+eps);
if mode == 'give',
    hi = max(max(pic));
    lo = min(min(pic));
    div = hi - lo;
    mult = length(cm) - 1;
    add = 1;
    sub = lo;
elseif mode ~= 'take',
    error('INVALID MODE; MODE SHOULD BE give OR take');
end

% take abs, and use image to display results
if nargout <= 4
    newplot;
    pic = (pic - sub) / div * mult + add;
    colormap(cm); 
    if length(t)==1
        image([0 1/f(2)],f,pic);axis xy; colormap(jet)
    else
        image(t,f,pic);axis xy; colormap(jet)
    end
    xlabel('Time')
    ylabel('Frequency')
elseif nargout == 5,
    yo = y;
elseif nargout == 6,
    yo = y;
    fo = f;
elseif nargout == 3,
    yo = y;
    fo = f;
    to = t;
end

function [msg,x,nfft,Fs,window,noverlap] = specgramchk(P)
%SPECGRAMCHK Helper function for SPECGRAM.
%   SPECGRAMCHK(P) takes the cell array P and uses each cell as 
%   an input argument.  Assumes P has between 1 and 5 elements.

msg = [];

x = P{1}; 
if (length(P) > 1) & ~isempty(P{2})
    nfft = P{2};
else
    nfft = min(length(x),256);
end
if (length(P) > 2) & ~isempty(P{3})
    Fs = P{3};
else
    Fs = 2;
end
if length(P) > 3 & ~isempty(P{4})
    window = P{4}; 
else
    if length(nfft) == 1
        window = hanning(nfft);
    else
        msg = 'You must specify a window function.';
    end
end
if length(window) == 1, window = hanning(window); end
if (length(P) > 4) & ~isempty(P{5})
    noverlap = P{5};
else
    noverlap = ceil(length(window)/2);
end

% NOW do error checking
if (length(nfft)==1) & (nfft<length(window)), 
    msg = 'Requires window''s length to be no greater than the FFT length.';
end
if (noverlap >= length(window)),
    msg = 'Requires NOVERLAP to be strictly less than the window length.';
end
if (length(nfft)==1) & (nfft ~= abs(round(nfft)))
    msg = 'Requires positive integer values for NFFT.';
end
if (noverlap ~= abs(round(noverlap))),
    msg = 'Requires positive integer value for NOVERLAP.';
end
if min(size(x))~=1,
    msg = 'Requires vector (either row or column) input.';
end

