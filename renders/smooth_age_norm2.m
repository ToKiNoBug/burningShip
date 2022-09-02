function [age,age_negative_1_index] = smooth_age_norm2(age,norm2)
    age_negative_1_index=(age<=-1);
    age=double(age);
    phi=log2(norm2)-1;
    mu=log2(abs(phi));
    age=age+1-mu;

    age(age_negative_1_index)=-1;


end

