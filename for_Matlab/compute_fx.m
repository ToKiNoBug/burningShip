function [f] = compute_fx(mat,varargin)
%COMPUTE_FX 此处显示有关此函数的摘要
%   此处显示详细说明
if nargin<=1
    x_length=32768;
else
    x_length=varargin{1};
end

f=int32(zeros(1,x_length));

for i=1:size(mat,1)*size(mat,2)
    if(mat(i)>=0)
        f(1+mat(i))=f(1+mat(i))+1;
    end
end

f=double(f)+1e-2;

f=f/sum(f);


end

