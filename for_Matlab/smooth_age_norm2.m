function [age,age_negative_1_index,normalized] = smooth_age_norm2(age,norm2)
    age_negative_1_index=(age<=-1);
    age=double(age);
    
    log2_norm_z=log2(norm2)/2;
    
    log2_log2_norm_z=log2(abs(log2_norm_z)); % in range (1,log2(log2(6))
    
    %max_lg2lg2=log2(log2(6));
    
    normalized=(log2_log2_norm_z)/(log2(log2(6))); % in range(0,1)
    
    age=age+1-normalized;

    age(age_negative_1_index)=1e9;
    
    age=log10(age);
    
    age(age_negative_1_index)=min(age,[],'all');
    
    normalized(age_negative_1_index)=sqrt(norm2(age_negative_1_index)/4);

end

