function [L] = expexp(x,k,m)
%EXPEXP 此处显示有关此函数的摘要
%   此处显示详细说明
L=1-exp(-m*x.*(1-exp(-k*x)));
end