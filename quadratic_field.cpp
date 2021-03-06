#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <assert.h>
#include <numeric>
#include <set>
#include "basic_functions.hpp"

//d:平方因子を持たず，0でも1でもない整数
//Remainder関数（命題3.1.6）：112行目
//Generator関数（命題3.2.2）に相当する関数：170行目　（ここではイデアルのコンストラクタとして実装してある）
//Contains関数：255行目　（ここでは，2つの生成元をGenerator関数により既に見つけてある場合について実装してある）
//PrimeFactorize関数：295行目

template<long long &d,typename T=long long>
struct ring_of_integer{
    struct elem{
        T a,b;
        elem():a(0),b(0){}
        elem(const T _a):a(_a),b(0){}
        elem(const T _a,const T _b):a(_a),b(_b){}
        elem operator-()const{
            return {-a,-b};
        }
        elem conjugate()const{
            return (MOD(d,4)==1 ? elem(a+b,-b):elem(a,-b));
        }
        T norm()const{
            return (*this * this->conjugate()).a;
        }
        friend bool operator==(const elem l,const elem r){
            return l.a==r.a && l.b==r.b;
        }
        friend bool operator!=(const elem l,const elem r){
            return !(l==r);
        }
        elem& operator+=(const elem r){
            a+=r.a;
            b+=r.b;
            return *this;
        }
        elem& operator-=(const elem r){
            this->a-=r.a;
            this->b-=r.b;
            return *this;
        }
        elem& operator*=(const elem r){
            if(MOD(d,4)==1){
                T old_a=this->a;
                this->a=this->a*r.a + this->b*r.b*(d-1)/4;
                this->b=this->b*r.b + this->b*r.a + old_a*r.b;
            }
            else{
                T old_a=this->a;
                this->a = this->a*r.a + this->b*r.b*d;
                this->b = old_a*r.b + this->b*r.a;
            }
            return *this;
        }
        friend elem operator+(elem l,const elem r){
            return l+=r;
        }
        friend elem operator-(elem l,const elem r){
            return l-=r;
        }
        friend elem operator*(elem l,const elem r){
            return l*=r;
        }
        friend std::ostream& operator<<(std::ostream &os,const elem r){
            if(r.a==0 && r.b==0)os << 0;
            else if(r.a==0)os << r.b << (MOD(d,4)==1 ? "\\frac{1+\\sqrt{":"\\sqrt{") << d << (MOD(d,4)==1 ? "}}{2}":"}");
            else if(r.b==0)os << r.a;
            else os << r.a << (r.b>0 ? "+":"") << r.b << (MOD(d,4)==1 ? "\\frac{1+\\sqrt{":"\\sqrt{") << d << (MOD(d,4)==1 ? "}}{2}":"}");
            return os;
        }
        bool is_divided_by(const elem r)const{
            assert(r!=0);
            elem prod=*this * r.conjugate();
            T div=r.norm();
            return prod.a%div==0 && prod.b%div==0;
        }
        bool is_divisor_of(const elem r)const{
            return r.is_divided_by(*this);
        }
        bool is_unit()const{
            return std::abs(this->norm())==1;
        }
        bool is_integer()const{
            return b==0;
        }
        friend void swap(elem &l,elem &r){
            const elem x=r;
            r=l;
            l=x;
            return;
        }
        // @return pair(u,v) s.t. {elem(x,y) | 0<=x<u, 0<=y<v} is a complete system of representatives in A/(*this).
        std::pair<T,T> mod_representative()const{
            assert((*this)!=0);
            const T n=std::abs(norm());
            T g;
            if(MOD(d,4)==1){
                g=std::gcd(a,(d-1)/4*b);
            }
            else{
                g=std::gcd(a,d*b);
            }
            return std::make_pair(g,n/g);
        }
        // @return t s.t. t==val in A/(r)
        friend elem Remainder(const elem val,const elem r){
            assert(r!=0);
            //R={a+b\alpha | 0<=a<u,u<=b<v}
            const auto [u,v]=r.mod_representative();
            const T s=r.a, t=r.b, n=std::abs(r.norm());//r=s+t\alpha
            T a=val.a, b=val.b;//x=a+b\alpha
            a%=n;if(a<0)a+=n;
            b%=v;if(b<0)b+=v;

            const T X=a/u*u;
            T Y_dt,Y_s,mod_dt,mod_s,g_dt,g_s;
            //solve (td/gcd(td,N(r)))Y \equiv Xs/gcd(td,N(r)) or (tD / gcd(tD,N(r)))Y \equiv -X(s+t)/gcd(tD,N(r)) (D=(1-d)/4)
            {
                const T D=(1-d)/4;
                T u,v;
                if(MOD(d,4)==1){
                    g_dt=std::gcd(t*D,n);
                    mod_dt=n/g_dt;
                    u=t*D/g_dt%mod_dt, v=-(s+t)*X/g_dt%mod_dt;
                }
                else{
                    g_dt=std::gcd(t*d,n);
                    mod_dt=n/g_dt;
                    u=t*d/g_dt%mod_dt, v=s*X/g_dt%mod_dt;
                }
                //solve uY=v mod(mod_dt)
                const T u_inv=solve_lineareq(u,mod_dt).first%mod_dt;
                Y_dt=v*u_inv%mod_dt;
                if(Y_dt<0)Y_dt+=mod_dt;
            }
            //solve (s/gcd(s,N(r)))Y \equiv Xt/gcd(s,N(r))
            {
                g_s=std::gcd(s,n);
                mod_s=n/g_s;
                const T u=s/g_s%mod_s, v=t*X/g_s%mod_s;
                //solve uY=v mod(mod_s)
                const T u_inv=solve_lineareq(u,mod_s).first%mod_s;
                Y_s=v*u_inv%mod_s;
                if(Y_s<0)Y_s+=mod_s;
            }

            //Y mod(lcm(mod_dt,mod_s)) i.e. mod(v)
            T Y=garner(Y_dt,mod_dt,Y_s,mod_s);

            //X+Y√d=0 mod(r)
            a-=X,b-=Y;
            if(b<0)b+=v;
            return elem(a,b);
        }
    };

    struct ideal{
        elem gen[2];
        ideal(){
            gen[0]=0;
            gen[1]=0;
        }
        //Fと同じイデアルを生成する2元を見つけ，gen[0]及びgen[1]とする
        ideal(std::vector<elem> F){
            std::vector<elem> tmp;
            for(elem val:F)if(val!=0)tmp.emplace_back(val);
            F=std::move(tmp);
            if(F.size()<=2ul){
                for(int i=0;i<(int)F.size();i++)gen[i]=F[i];
                return;
            }

            //xにノルムの絶対値が最小の元を入れる
            elem x=F.front();
            T n=std::abs(x.norm());
            for(elem val:F)if(n>std::abs(val.norm())){
                x=val;
                n=std::abs(val.norm());
            }

            //擬似コードにおけるIを実際に集合として管理する代わりに，「既に現れた」ことを示すbool値のvectorを用いる．
            //seen_I[a][b]=true <=> a+b\alphaがIに含まれる．
            //同時にIの大きさsize_Iも管理する．
            auto [u,v]=x.mod_representative();
            std::vector seen_I(u,std::vector<bool>(v));
            seen_I[0][0]=true;
            T size_I=1;
            std::queue<elem> q;
            q.emplace(0);

            while(!q.empty()){
                //qの先頭元をwとする
                elem w=q.front();
                q.pop();

                for(elem t:F)for(elem z:{Remainder(w+t,x),Remainder(w+t*elem(0,1),x)}){
                    //zがIに含まれない場合
                    if(!seen_I[z.a][z.b]){
                        seen_I[z.a][z.b]=true;
                        size_I++;
                        q.emplace(z);
                    }
                }
            }

            //Xの代わりにseen_Xを管理する
            std::vector seen_X(u,std::vector<bool>(v));
            for(T a=0;a<u;a++)for(T b=0;b<v;b++)if(seen_I[a][b] && !seen_X[a][b]){
                elem t=elem(a,b);
                //Jの代わりにseen_J，size_Jを管理する
                std::vector seen_J(u,std::vector<bool>(v));
                seen_J[0][0]=true;
                T size_J=1;
                q.emplace(0);

                while(!q.empty()){
                    //qの先頭元をwとする
                    elem w=q.front();
                    q.pop();
                    //最後にXにJの元を加える代わりに，ここでXにwを加える
                    seen_X[w.a][w.b]=true;

                    for(elem z:{Remainder(w+t,x),Remainder(w+t*elem(0,1),x)}){
                        //zがJに含まれない場合
                        if(!seen_J[z.a][z.b]){
                            seen_J[z.a][z.b]=true;
                            size_J++;
                            q.emplace(z);
                        }
                    }
                }
                if(size_I==size_J){
                    gen[0]=x;
                    gen[1]=t;
                    return;
                }
            }

            //ここに到達する前に終了しているべきである
            assert(false);
            return;
        }
        ideal operator+(const ideal &r)const{
            return ideal({gen[0],gen[1],r.gen[0],r.gen[1]});
        }
        ideal operator*(const ideal &r)const{
            return ideal({gen[0]*r.gen[0],gen[0]*r.gen[1],gen[1]*r.gen[0],gen[1]*r.gen[1]});
        }
        bool Contains(elem x){
            if(gen[0]==0)swap(gen[0],gen[1]);
            if(gen[0]==0)return x==0;

            //零でない，ノルムの小さい方の元をgen[0]とする．
            if(gen[1]!=0 && std::abs(gen[0].norm()) > std::abs(gen[1].norm()))swap(gen[0],gen[1]);
            gen[1]=Remainder(gen[1],gen[0]);
            auto [u,v]=gen[0].mod_representative();

            x=Remainder(x,gen[0]);

            if(x==0)return true;
            //Iを管理する代わりにseenを管理する
            std::vector seen(u,std::vector<bool>(v));
            seen[0][0]=true;
            std::queue<elem> q;
            q.emplace(0);

            while(!q.empty()){
                //qの先頭元をwとする．
                elem w=q.front();q.pop();
                for(elem z:{Remainder(w+gen[1],gen[0]),Remainder(w+gen[1]*elem(0,1),gen[0])}){
                    //xがIに含まれない場合
                    if(!seen[z.a][z.b]){
                        if(z==x)return true;
                        seen[z.a][z.b]=true;
                        q.emplace(z);
                    }
                }
            }
            //xがIに含まれるならば，ここまでのどこかでtrueがreturnされているはずである．
            return false;
        }
        bool Contains(ideal J){
            return Contains(J.gen[0]) && Contains(J.gen[1]);
        }
        bool operator==(ideal &r){
            return this->Contains(r) && r.Contains(*this);
        }
        // @return the vector of pairs(p,i) s.t. (*this) is a product of p^i.
        std::vector<std::pair<ideal,int>> PrimeFactorize(){
            auto [u,v]=gen[0].mod_representative();
            //X,Yを管理する代わりに，seen_X,seen_Yという配列を管理する．
            std::vector seen_X(u,std::vector<bool>(v)),seen_Y(u,std::vector<bool>(v));
            for(T i=0;i<u;i++)for(T j=0;j<v;j++){
                //tがXの要素なら次へ．
                if(seen_X[i][j])continue;
                else{
                    elem t(i,j);
                    //I,seen_Iを同時に管理する．
                    std::vector<elem> I={0};
                    std::vector seen_I(u,std::vector<bool>(v));
                    seen_I[0][0]=true;
                    std::queue<elem> q;
                    q.emplace(0);
                    while(!q.empty()){
                        auto w=q.front();q.pop();
                        for(elem z:{Remainder(w+t,gen[0]),Remainder(w+t*elem(0,1),gen[0])}){
                            //zがIに含まれない時
                            if(!seen_I[z.a][z.b]){
                                seen_I[z.a][z.b]=true;
                                I.emplace_back(z);
                                q.emplace(z);
                            }
                        }
                    }
                    //Iが全体に一致した場合は次の元へ．
                    if((T)I.size()==std::abs(gen[0].norm()))continue;
                    for(elem w:I){
                        seen_X[w.a][w.b]=true;
                        seen_Y[w.a][w.b]=false;
                    }
                    seen_Y[t.a][t.b]=true;
                }
            }
            std::vector<std::pair<ideal,int>> S;
            //Yの要素を走査する
            for(T i=0;i<u;i++)for(T j=0;j<v;j++)if(seen_Y[i][j]){
                elem t(i,j);
                int cnt=0;
                ideal J({gen[0],t});
                while(J.Contains(*this)){
                    J=J*ideal({gen[0],t});
                    cnt++;
                }
                if(cnt)S.push_back({ideal({gen[0],t}),cnt});
            }
            return S;
        }
    };
};

bool is_prime(int p){
    for(int i=2;i*i<=p;i++){
        if(p%i==0)return false;
    }
    return true;
}

using std::cout;
using std::endl;

long long d=-5;

//利用例．これは例3.3.4で実行したものと同じである．
int main(){
    using A=ring_of_integer<d>;
    for(int p=2;p<=100;p++)if(is_prime(p)){
        A::ideal I({p});
        auto res=I.PrimeFactorize();
        //Iが素イデアルならばpを出力する．
        if(res.size()==1ul && res[0].second==1){
            cout << p << endl;
        }
    }
}
