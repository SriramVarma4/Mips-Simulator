#include<iostream>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
using namespace std;


//debug
int dbgn =0;
void debug(){
    cout<<"##Debug:"<<dbgn++<<"##\n";
}



//important constants
const int MemorySize = 1048576;
const int numOfReg = 32;
const string reglist[32] = {
        "z  ",
        "at ",
        "v0 ","v1 ",
        "a0 ","a1 ","a2 ","a3 ",
        "t0 ","t1 ","t2 ","t3 ","t4 ","t5 ","t6 ","t7 ",
        "s0 ","s1 ","s2 ","s3 ","s4 ","s5 ","s6 ","s7 ",
        "t8 ","t9 ",
        "k0 ","k1 ",
        "gp ","sp ","fp ",
        "ra " 
};

typedef bitset<8>  byte8;
typedef bitset<32> byte32;


byte8 mem[MemorySize];
byte32 reg[numOfReg];
int cycles=0;
class Mem{

    public:

    map <string,int> registerMap; 
    vector<int> linepos;
    
    int Memstart;

    private:
    bool flag = true;

    public:

    Mem(string code){

        int addr = 0;

        //parses into instructions
        linepos.push_back(0);
        for(int i=0;i<code.length();i++){
            if(code[i] == ' ' || code[i] == ',' || code[i] == '\n'){
                int k = addr%4;
                if(k>0){
                    while(k<4){
                        mem[addr] = (byte8)32;
                        k++;
                        addr++;
                    }
                }
                if(code[i] == '\n'){
                    linepos.push_back(addr);
                }
            }
            else{
                mem[addr] = (byte8)code[i];
                addr++;
            }
        }
        int k = addr%4;
        if(k>0){
            while(k<4){
                mem[addr] = (byte8)32;
                k++;
                addr++;
            }
        } 
        Memstart = addr;

        //adds registers in Register array
        for(int i=0;i<32;i++){
            registerMap[reglist[i]] = i;
        }
    }

    void execute(int ex){ 
        if(ex<Memstart){ 
            cout<<"\nExecuting Memory position : "<<ex<<endl;
            cycles++;
            if(ex%4==0){
                int token = findtoken(ex);
                int nextex = ex+16;
                switch (token){
                           //add
                    case 1:{
                                int val1=getreg(ex+4);
                                int val2=getreg(ex+8);
                                int val3=getreg(ex+12);
                                reg[val1] = (byte32)(reg[val2].to_ulong()+reg[val3].to_ulong());
                                break;
                           }
                           //sub
                    case 2:{
                                int val1=getreg(ex+4);
                                int val2=getreg(ex+8);
                                int val3=getreg(ex+12);
                                reg[val1] = (byte32)(reg[val2].to_ulong()-reg[val3].to_ulong());
                                break;
                           }
                           //mul
                    case 3:{
                                int val1=getreg(ex+4);
                                int val2=getreg(ex+8);
                                int val3=getreg(ex+12);
                                reg[val1] = (byte32)(reg[val2].to_ulong()*reg[val3].to_ulong());
                                break;
                           }
                           //beq
                    case 4:{
                                int val1=getreg(ex+4);
                                int val2=getreg(ex+8);
                                if(reg[val1]==reg[val2])
                                    nextex = getloc(ex+12);
                                break;
                           }
                           //bne
                    case 5:{
                                int val1=getreg(ex+4);
                                int val2=getreg(ex+8);
                                if(reg[val1]!=reg[val2])
                                    nextex = getloc(ex+12);
                                break;
                           }
                           //slt
                    case 6:{
                                int val1=getreg(ex+4);
                                int val2=getreg(ex+8);
                                int val3=getreg(ex+12);
                                if(reg[val1].to_ulong()<reg[val2].to_ulong())
                                    reg[val3] = (byte32)1;
                                else
                                    reg[val3] = (byte32)0;
                                break;
                           }
                           //j
                    case 7:{
                               nextex = getloc(ex+4);
                               break;
                           }
                           //lw
                    case 8:{
                               int val1 = getreg(ex+4);
                               int val2 = getreg(ex+8);
                               reg[val1] = (byte32)(mem[Memstart+val2].to_string()+mem[Memstart+val2+1].to_string()+mem[Memstart+val2+2].to_string()+mem[Memstart+val2+3].to_string());
                               nextex = ex+12;
                               break;
                           }
                           //sw
                    case 9:{
                               int val1 = getreg(ex+4);
                               int val2 = getreg(ex+8);
                               string memval = reg[val1].to_string();
                               for(int i=0;i<4;i++){ 
                                   mem[Memstart+val2+i] = (byte8)(memval.substr(i*8,8));
                               }
                               nextex = ex+12;
                               break;
                           }
                           //addi
                    case 10:{
                                int val1 = getreg(ex+4);
                                int val2 = getreg(ex+8);
                                reg[val1] = (byte32)(reg[val2].to_ulong() + getval(ex+12));
                                break;
                            }
                }
                if(flag){
                    printregs();
                    execute(nextex);
                }
            }
        }
    }

    int findtoken(int pos){
        int ret;
        if(pos>=Memstart){
            cout<<"ERROR, token not found at pos "<<pos<<endl;
            flag = false;
        }
        else{
        string word;
        for(int i=pos;i<pos+4;i++){
            word = word + (char)(mem[i].to_ulong());
        }
        if(word == "add ") ret=1;
        else if(word == "sub ") ret=2;
        else if(word == "mul ") ret=3;
        else if(word == "beq ") ret=4;
        else if(word == "bne ") ret=5;
        else if(word == "slt ") ret=6;
        else if(word == "j   ") ret=7;
        else if(word == "lw  ") ret=8;
        else if(word == "sw  ") ret=9;
        else if(word == "addi") ret=10;
        else {
            cout<<"ERROR, token not found at pos "<<pos<<endl;
            flag = false;
        } //raise error 

        }

        return ret;
    }

    int getreg(int pos){
        int loc;
        if(pos>=Memstart){
            cout<<"ERROR, token not found at pos "<<pos<<endl;
            flag = false;
        }
        else{
        if((char)mem[pos].to_ulong() == '$'){
            string word;
            for(int i=pos+1;i<pos+4;i++){
                word = word + (char)(mem[i].to_ulong());
            }
            loc = registerMap.find(word)->second; 
        }

        }
        if(loc >= 32){
            cout<<"ERROR, register not found at pos "<<pos<<endl;
            flag = false;
        }
        return loc;
    }
    int getloc(int pos){
        int loc;
        if(pos>=Memstart){
            cout<<"ERROR, token not found at pos "<<pos<<endl;
            flag = false;
        }
        else{
        loc = getval(pos);
        if(loc>= linepos.size()){
            cout<<"ERROR, no line "<<loc<<" from pos "<<pos<<endl;
            flag = false;
        }
        }
        return linepos[loc];
    }

    void printregs(){
        cout<<"\nRegisters:\n";
        for(int i=0;i<32;i++){
            cout<<"register "<<reglist[i]<<" : "<<hex<<reg[i].to_ulong()<<endl<<dec;
        }
    }
    void printcode(){
        for(int i=0;i<Memstart;i++){
            cout<<"Mem "<<i<<" : "<<(char)mem[i].to_ulong()<<endl;
        }
    }
    int getval(int pos){
        string word;
        for(int i=pos;i<pos+4;i++){
            char ab = (char)(mem[i].to_ulong());
            if((int)ab<48 || (int)ab>57){
               cout<<"ERROR, invalid integer at pos "<<pos<<endl; 
               flag = false;
               break;
            }
            word = word + ab ;
        }
        return stoi(word);
    }
};

int main(int argc, char* argv[]){
    //take input from file and make a new object with it
    //execute the object once
    string code,line;
    ifstream file;
    file.open(argv[1]);
    if(file.is_open()){
        while(getline(file,line)){
            code+="\n"+line;
        }
    }
    //reg[0] = (byte32)10;
    Mem obj(code);
    cout<<"\nText File in Memory:\n";
    obj.printcode();
    obj.execute(0);
    cout<<"\nNo. of Cycles executed : "<<cycles<<endl;
    return 0; 
}
