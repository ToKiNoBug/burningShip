## 基于分布熵的渲染方法


首先确定像素亮度L与迭代次数x的关系：
$$
L=\frac{2}{\pi}\cdot \arctan{(qx^2)}
$$

其中q为一参数。

不难统计出x的分布函数$f_x(x)$（本质上是离散分布，但由于x取值范围广，可以近似为连续分布）

由于L与x的关系单调递增，L的分布函数
$$
f_L(L)=f_x(x(L))\cdot \frac{dx}{dL}=f_x\cdot \frac{dx}{dL}
$$

根据分布熵的一种不严格定义，有
$$
H_L=-\int f_L\cdot \ln{f_L} dL
$$

因此
$$
H_L=-\int f_x\cdot \ln{f_L}\cdot \frac{dx}{dL} dL=-\int \ln{f_L}\cdot f_x dx
$$

容易知道
$$
\frac{dx}{dL}=(\frac{dL}{dx})^{-1}=\frac{\pi}{4}\frac{1+q^2x^4}{qx}
$$

则
$$
\ln{f_L}=\ln{f_x}+\ln\frac{\pi}{4}+\ln{(\frac{\pi}{4}\frac{1+q^2x^4}{qx})}
$$

因此
$$
H_L=-\int (\ln{f_x}+\ln\frac{\pi}{4})f_xdx-\int f_x\cdot\ln{\frac{1+q^2x^4}{qx}}dx
$$

式子中第一项不含q，计算它对调节L的分布熵没有意义，定义
$$
h_L(q)=-\int f_x\cdot\ln{\frac{1+q^2x^4}{qx}}dx
$$

为便于数值计算，略微修改$h_L(q)$的定义，得到
$$
\eta_L(q)=-\sum_{x=1}^{x_{max}} f_x\cdot\ln{\frac{1+q^2x^4}{qx}}
$$

找到使$\eta_L(q)$最大的$q_m$，渲染时取$q=0.1q_m$。


容易证明
$$
\frac{d\eta_L}{d\ln{q}}=\sum_{x=1}^{x_{max}}f_x\cdot \frac{1-q^2x^4}{1+q^2x^4}
\\
\frac{d\eta_L}{d\ln{q}} \Big|_{q\to0^+}=\sum_{x=1}^{x_{max}}f_x=1-f_x(0)
\\
\frac{d\eta_L}{d\ln{q}} \Big|_{q\to+\infty}=-\sum_{x=1}^{x_{max}}f_x=f_x(0)-1
$$

则$q\to0^+$时，渐近线
$$
\eta_L=(1-f_x(0))\cdot \ln{q}+\sum_{x=1}^{x_{max}}f_x\ln{x}
$$

$q\to+\infin$时，渐近线
$$
\eta_L=(f_x(0)-1)\cdot \ln{q}-3\sum_{x=1}^{x_{max}}f_x\ln{x}
$$

两条渐近线交点对应的$q$值可以作为最大值点的估计值：
$$
\ln{q}=-2\frac{\sum_{x=1}^{x_{max}}f_x\ln{x}}{1-f_x(0)}
$$

则求最大值点的牛顿迭代式
$$
\ln{q} \leftarrow \ln{q}-\frac{\sum f_x\cdot\frac{1-q^2x^4}{1+q^2x^4}}{\sum f_x\cdot\frac{-4q^2x^4}{1+q^2x^4}}
$$