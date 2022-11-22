function [matL] = compute_matL(mat_age,q,varargin)
%COMPUTE_MATL 此处显示有关此函数的摘要
%   此处显示详细说明
matL=double(mat_age);

matL=2/pi*atan(q*matL.^2);

matL(mat_age<0)=0;
end

