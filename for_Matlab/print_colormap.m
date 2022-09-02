function [str] = print_colormap(fn,L,varargin)

str="{";

list=single(fn(L));

for r=1:L
   str=strcat(str,'{',num2str(list(r,1),128),',' ,num2str(list(r,2),128),',',num2str(list(r,3),128),'},');
end
str=strcat(str,'}');

if nargin>=3
   fid=fopen(varargin{1},"wb");
   fwrite(fid,str);
   fclose(fid);   
end

end

