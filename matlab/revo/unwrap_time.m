function t = unwrap_time(t)

wrap = find((t(2:end) - t(1:end-1) ) < -64000);
jump = zeros(size(t));
jump(wrap+1) = hex2dec('10000');

t = t + cumsum(jump);