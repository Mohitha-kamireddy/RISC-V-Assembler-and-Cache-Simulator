#include "assembler.hpp"
long binToDec(const std::string &bin);
std::string decToBin(long dec);
void Assembler::up(string &s) {
      for (char &c : s) c = toupper(c);
}

long binToDec(const string &bin){
    if (bin.empty()) return 0;
    long val=0;
    int n = bin.size();
    bool isNegative = (bin[0]=='1');
    for (int i=0; i<n; ++i){
        val = (val<<1)|(bin[i]-'0');
    }
    
    if (isNegative) {val -= (1L<<n);}
    return val;
}

string decToBin(long dec){
    string res ="";
    unsigned int uval = static_cast<unsigned int>(dec);
    for(int i=0; i<32; ++i){
        res=((uval & 1) ? '1' : '0') + res, uval>>=1;
    }
    return res;
}
int Assembler::toInt(const string &s){
    int sign = 1, i =0 , res = 0;
    if(!s.empty() && s[0] == '-'){
        sign = -1 ; i=1;
    }
    for (; i< (int)s.size();++i) res=res*10+(s[i]-'0');
    return res*sign;
    }
    
    string Assembler::toBin(int num){
        string res;
        unsigned int val = static_cast<unsigned int>(num);
        for (int i=0; i<32; ++i)
        res += (val&(1<<(31-i)))?'1':'0';
        return res;
    }
    
    string Assembler::immBin(string imm, int bits){
        int val = toInt(imm);
        string bin = toBin(val);
        return bin.substr(32-bits);
   }
   
   string Assembler::bin5(int n){
       string b = toBin(n);
       return b.substr(27,5);
   }
   
   void Assembler::makeMaps(FuncTypes &F){
       F.R["ADD"]=0;
       F.R["SUB"]=10;
       F.R["MUL"]=11;
       F.R["DIV"]=12;
       F.R["REM"]=13;
       F.R["SLT"]=2;
       
       F.I["ADDI"]=0;
       
       F.B["BEQ"]=0;
       F.B["BLT"]=4;
       
       F.J["JAL"]=-1;
       F.S["SW"] = 0;
       
   }
   
   string Assembler:: rtype(vector<string> tokens, FuncTypes &F){
       string opcode = "0110011";
       
       int rd = stoi(tokens[1].substr(1));
       int rs1 = stoi(tokens[2].substr(1));
       int rs2 = stoi(tokens[3].substr(1));
       
       string funct7, funct3;
       string op = tokens[0];
       for (auto &c : op) c = toupper(c);
       
       if (op =="ADD") {funct7 = "0000000"; funct3 = "000";}
       else if (op =="SUB") {funct7 = "0100000"; funct3 = "000";}
       else if (op =="SLT") {funct7 = "0000000"; funct3 = "010";}
       else if (op =="MUL") {funct7 = "0000001"; funct3 = "000";}
       else if (op =="DIV") {funct7 = "0000001"; funct3 = "100";}
       else if (op =="REM") {funct7 = "0000001"; funct3 = "110";}
       else return "";
       return funct7 + bin5(rs2) + bin5(rs1) + funct3 + bin5(rd) + opcode;
       
   }
   
   string Assembler::itype(vector<string> tokens, FuncTypes &F){
       string opcode ="0010011";
       string funct3="000";
       int rd = stoi(tokens[1].substr(1));
       int rs1 = stoi(tokens[2].substr(1));
       string imm=immBin(tokens[3],12);
       return imm + bin5(rs1) + funct3 + bin5(rd) +opcode;
   }
   
   string Assembler::btype(vector<string> tokens, FuncTypes &F){
       string opcode = "1100011";
       string funct3 = bin5(F.B[tokens[0]]).substr(2);
       int rs1 = stoi(tokens[1].substr(1));
       int rs2 = stoi(tokens[2].substr(1));
       string imm = immBin(tokens[3],13);
       
       string imm12 = string(1, imm[0]);
       string imm10_5 = imm.substr(2,6);
       string imm4_1 = imm.substr(8,4);
       string imm11 = string(1, imm[1]);
       
       return imm12 + imm10_5 + bin5(rs2) + bin5(rs1) + funct3 + imm4_1 + imm11 + opcode;
       
       
   }
   
    string Assembler::jtype(vector<string> tokens, FuncTypes &F){
         string opcode = "1101111";
         int rd = stoi(tokens[1].substr(1));
         int imm = stoi(tokens[2]);
         
         string immBinStr = immBin(to_string(imm),21);
         string b20 = string(1,immBinStr[0]);
         string b10_1 = immBinStr.substr(11,10);
         string b11 = string (1, immBinStr[10]);
         string b19_12 = immBinStr.substr(1,8);
         
         string enc = b20+b10_1+b11+b19_12;
         return enc + bin5(rd) + opcode;
         
     }
     
    string Assembler::stype(vector<string> tokens, FuncTypes &F){
        string opcode = "0100011";
        int rs2 = stoi(tokens[1].substr(1));
        
        auto l = tokens[2].find('('), r = tokens[2].find(')');
        int imm = stoi(tokens[2].substr(0,l));
        int rs1 = stoi(tokens[2].substr(l+2, r- (l+2)));
          string immBinStr = immBin(to_string(imm),13);
          string imm11_5 = immBinStr.substr(0,7);
          string imm4_0 = immBinStr.substr(7,5);
          string funct3 = "010";
          
          return imm11_5 + bin5(rs2) + bin5(rs1) + funct3 + imm4_0 + opcode;
    }
    
    string Assembler::parse(vector<string> tokens, FuncTypes &F){
        up(tokens[0]);
        if (F.R.count(tokens[0])) return rtype(tokens, F);
        if (F.I.count(tokens[0])) return itype(tokens, F);
        if (F.B.count(tokens[0])) return btype(tokens, F);
        if (F.J.count(tokens[0])) return jtype(tokens, F);
        if (F.S.count(tokens[0])) return stype(tokens, F);
        return"";
        }
    
    vector<string> Assembler:: Tokenize(const string &line){
        vector<string> tokens;
        string token;
        for (char ch:line){
            if ( ch==',' || ch == ' ' || ch=='\t' || ch == ';'){
                if (!token.empty()) tokens.push_back(token);
                token.clear();
            }else{
                token += ch;
            }
        }
        if (!token.empty()) tokens.push_back(token);
        return tokens;
    }
    
    
    vector<string> Assembler::assemble(){
        string input, fullInput;
        FuncTypes F;
        makeMaps(F);
        cout << "Enter assemnly code (end each instruction with ';')" << endl;
        
        while (true){
            getline(cin,input);
            if (input.empty())break;
            fullInput += input + " ";
        }
        
        vector<string> binlines;
        stringstream ss(fullInput);
        string line;
        while (getline(ss, line, ';')){
            vector<string> tokens = Tokenize(line);
            if (!tokens.empty()){
                string bin = parse(tokens,F);
                if(!bin.empty()){
                    cout << bin <<endl;
                    binlines.push_back(bin);
                }
                
            }
        }
        return binlines;
    }
    
    long Assembler::binToDec(const string &bin){
        if(bin.empty()) return 0;
        long val =0 ;
        int n= bin.size();
        bool isNegative = (bin[0]=='1');
        for (int i=0; i<n; ++i){
            val = (val<<1) | (bin[i] -'0');
            
        }
        
        if (isNegative){
            val -= (1L<<n);
        }
        return val;
    }
    
    string Assembler::decToBin(long dec){
        string res = "";
        unsigned int uval = static_cast<unsigned int>(dec);
        for(int i =0; i<32 ;++i){
            res = ((uval & 1) ? '1': '0') + res , uval >>=1 ;
        }
        return res;
    }
    
    struct controlword{
    bool AluSrc = false , Mem2Reg = false , RegWrite = false , MemRead = false , MemWrite = false, Branch = false , Jump = false ;
    } ;
    controlword maincontrol(const string &opcode){
        controlword CW;
        if(opcode == "0110011") {CW.RegWrite = true ;}
        else if (opcode == "0010011") { CW.RegWrite = true ; CW.AluSrc = true ; }
        else if (opcode == "0100011") { CW.MemWrite = true ; CW.AluSrc = true ; }
        else if (opcode == "1100011") { CW.Branch = true ; }
        else if (opcode == "1101111") { CW.RegWrite = true ; CW.Jump = true ;}
        else if (opcode == "0000011") { CW.RegWrite = true ; CW.AluSrc = true ; CW.Mem2Reg = true ; CW.MemRead = true;}
        else if (opcode == "0100011") { CW.MemWrite = true ; CW.AluSrc = true ; }
        return CW;
    }
    
    long immGen(const string &inst , const string &opcode){
        if(opcode == "0010011"){
            return binToDec(inst.substr(0,12));
        }
        else if (opcode == "0100011"){ 
            string imm_hi = inst.substr(0,7) , imm_lo = inst.substr(20,5);
            return binToDec(imm_hi + imm_lo);
        }
        else if (opcode == "1100011"){ 
            string imm12 = inst.substr(0,1) , imm10_5 = inst.substr(1,6), imm11 = inst.substr(24,1) , imm4_1 = inst.substr(20,4);
            string imm = imm12+imm11+imm10_5 + imm4_1 + "0";
            return binToDec(imm) ;
        }
        else if (opcode == "1101111"){ 
            string imm20 = inst.substr(0,1) , imm10_1 = inst.substr(1,10), imm11 = inst.substr(11,1) , imm19_12 = inst.substr(12,8);
            string imm = imm20 +imm19_12+imm11 + imm10_1 + "0";
            return binToDec(imm) ;
        }
        return 0;
    }
    
    string aluControl(const string &opcode , const string &funct7 , const string &funct3){
        if(opcode == "0110011"){
            if(funct7 == "0000000" && funct3 == "000") return "ADD";
            else if(funct7 == "0100000" && funct3 == "000") return "SUB";
            else if(funct7 == "0000001" && funct3 == "000") return "MUL";
            else if(funct7 == "0000001" && funct3 == "100") return "DIV";
            else if(funct7 == "0000001" && funct3 == "110") return "REM";
            else if(funct7 == "0000000" && funct3 == "010") return "SLT";
        }
        else if(opcode == "0010011" ) return "ADD";
        return "NOP";
    }
    
    long Mux_AluSrc(bool AluSrc, long val_rs2, long imm){
        if(AluSrc) return imm;
        else return val_rs2;
    }
    
    long Mux_WriteBack(bool write_pc4, long aluOut, long pc_plus4){
        if(write_pc4) return pc_plus4;
        else return aluOut;
    }
    
    size_t Mux_NextPC(bool branch_taken, size_t currentPC, size_t target){
        if(branch_taken) return target;
        else return currentPC +1;
    }
    
    void singleCycleProcessor(const vector<string>& binaries){
         vector<string> GPR(32,string(32,'0'));
         for(size_t pc = 0; pc<binaries.size(); ++pc){
             const string &inst=binaries[pc];
             string opcode=inst.substr(25,7);
             string funct7=inst.substr(0,7);
             string funct3=inst.substr(17,3);
             int rs2=binToDec('0'+inst.substr(7,5));
             int rs1=binToDec('0'+inst.substr(12,5));
             int rd=binToDec('0'+inst.substr(20,5));
             
             controlword CW = maincontrol(opcode);
             long imm = immGen(inst,opcode);
             string aluOp= aluControl(opcode, funct7, funct3);
             
             long op1= binToDec(GPR[rs1]);
             long op2 = CW.AluSrc ? imm: binToDec(GPR[rs2]);
             long res =9;
             
             if(aluOp=="ADD") res = op1+op2;
             else if(aluOp=="SUB") res = op1-op2;
             
             if(CW.RegWrite && rd!=0) GPR[rd]=decToBin(res);
             GPR[0]=string(32,'0');
             
             cout<<"PC="<<pc<<": ";
             for(int i=0; i<32; i++) cout <<"x"<<i<<"="<<binToDec(GPR[i])<<" ";
             cout<<endl;
         }
         
         cout<<"\nFinal Registers: |n";
          for(int i=0; i<32; i++) cout <<"x"<<i<<"="<<binToDec(GPR[i])<<"\n";
         
    }
    
    void Assembler::singlecycleprocessor(const vector<string>&binaries){
        vector<string> GPR(32,string(32,'0'));
        vector<string> DataMem(32,string(32,'0'));
        
        for(size_t pc = 0; pc<binaries.size(); ++pc){
        const string &inst = binaries[pc];
        if(inst.empty()) continue;
        string opcode = inst.substr(25,7);
        controlword CW = maincontrol(opcode);
        
        if(opcode == "0110011"){
            string funct7 = inst.substr(0,7);
            int rs2 = binToDec("0" + inst.substr(7,5));
            int rs1 = binToDec("0" + inst.substr(12,5));
            string funct3 = inst.substr(17,3);
            int rd = binToDec("0" + inst.substr(20,5));
            long val_rs1 = binToDec(GPR[rs1]);
            long val_rs2 = binToDec(GPR[rs2]);
            long result = 0;
            if (funct7 == "0000000" && funct3 == "000")result= val_rs1 + val_rs2;
            else if (funct7 =="0100000" && funct3 =="000")result= val_rs1 - val_rs2;
            if (CW.RegWrite && rd!= 0){
                GPR[rd]=decToBin(result);
            }
        }
        else if (opcode == "0010011"){
            int imm = binToDec(inst.substr(0,12));
            int rs1 = binToDec("0" + inst.substr(12,5));
            string funct3 = inst.substr(17,3);
            int rd = binToDec("0" + inst.substr(20,5));
            long val_rs1 = binToDec(GPR[rs1]);
            long result = val_rs1 + imm;
             if (CW.RegWrite && rd!= 0){
                GPR[rd]=decToBin(result);
            }
        }
        else if (opcode =="0000011"){
            int imm = binToDec(inst.substr(0,12));
            int rs1 = binToDec("0" + inst.substr(12,5));
            int rd = binToDec("0" + inst.substr(20,5));
            int addr = binToDec(GPR[rs1]);
            if (CW.MemRead && addr>=0 && addr <(int)DataMem.size() && rd!=0){
                GPR[rd]=DataMem[addr];
            }
        }
         else if (opcode =="0100011"){
            int imm = binToDec(inst.substr(0,7)+inst.substr(20,5));
            int rs1 = binToDec("0" + inst.substr(12,5));
            int rs2 = binToDec("0" + inst.substr(7,5));
            int addr = binToDec(GPR[rs1])+imm;
            if (CW.MemWrite && addr>=0 && addr <(int)DataMem.size()){
                DataMem[addr]=GPR[rs2];
            }
        }
        cout << "PC=" << pc << ": ";
        for (int i=0;i<32;++i){
            cout << "x"<<i<<"="<<binToDec(GPR[i])<<" ";
        }
        cout << endl;
    }
    }
    
    void fivestagepipeline(const vector<string> &binaries){
        vector<string> GPR(32,string(32,'0'));
        
        struct IF_ID{bool valid=false;size_t pc=0; string inst="";}IFID, nextIFID;
        struct ID_EX{bool valid=false;controlword CW; int rs1=0,rs2=0,rd=0; long val_rs1=0,val_rs2=0,imm=0;
        string opcode,funct7,funct3;size_t pc=0;} IDEX, nextIDEX;
        struct EX_MEM{bool valid=false;controlword CW; int rd=0; long aluOut=0;long val_rs2=0;
        bool branch_taken=false; size_t target=0; bool write_pc4=false; long pc_plus4=0;} EXMEM ,nextEXMEM;
        struct MEM_WB{bool valid=false; controlword CW; int rd=0; long aluOut=0; bool write_pc4=false; long pc_plus4=0;} MEMWB, nextMEMWB;
        
        size_t PC=0; int cycle=1;
        
        auto printRegs = [&](const vector<string>&regs){
            cout <<"Registers: ";
            for (int i=0; i<32; ++i) cout <<"x" << i << binToDec(regs[i]) << " ";
            cout <<endl;
        };
        
        while (PC<binaries.size()|| IFID.valid||IDEX.valid||EXMEM.valid||MEMWB.valid){
            cout <<"\ncycle"<<cycle++<<":\n";
            
            if(MEMWB.valid && MEMWB.CW.RegWrite && MEMWB.rd!=0){
                long wbData= Mux_WriteBack(MEMWB.write_pc4, MEMWB.aluOut, MEMWB.pc_plus4);
                GPR[MEMWB.rd]=decToBin(wbData);
            }
            
            nextMEMWB = {};
            nextEXMEM = {};
            nextIDEX = {};
            nextIFID = {};
            
           
            if (EXMEM.valid){
                nextMEMWB.valid=true;
                nextMEMWB.CW=EXMEM.CW;
                nextMEMWB.rd=EXMEM.rd;
                nextMEMWB.aluOut=EXMEM.aluOut;
                nextMEMWB.write_pc4=EXMEM.write_pc4;
                nextMEMWB.pc_plus4=EXMEM.pc_plus4;
            }
            
            bool branch_now = false;
            size_t branch_target =0;
            
           
            if(IDEX.valid){
                nextEXMEM.valid=true;
                nextEXMEM.CW=IDEX.CW;
                nextEXMEM.rd=IDEX.rd;
                nextEXMEM.val_rs2= IDEX.val_rs2;
                
                long op1 = IDEX.val_rs1;
                long op2=IDEX.val_rs2;
                if(EXMEM.valid && EXMEM.CW.RegWrite && EXMEM.rd==IDEX.rs1 && EXMEM.rd!=0) op1= EXMEM.aluOut;
                if(EXMEM.valid && EXMEM.CW.RegWrite && EXMEM.rd==IDEX.rs2 && EXMEM.rd!=0) op2= EXMEM.aluOut;
                
                long aluB = Mux_AluSrc(IDEX.CW.AluSrc, op2, IDEX.imm);
                string aluOp = aluControl(IDEX.opcode, IDEX.funct7, IDEX.funct3);
                long res=0;
                if(aluOp=="ADD") res= op1+aluB;
                else if(aluOp=="SUB") res = op1- aluB;
                else if (aluOp=="MUL") res = op1*aluB;
                else if(aluOp=="DIV") res = (aluB!=0)? op1/aluB:0;
                else if(aluOp=="REM") res = (aluB!=0)? op1%aluB: 0;
                else if (aluOp=="SLT") res = (op1<aluB)?1:0;
                
                nextEXMEM.aluOut=res;
               
                
                if(IDEX.CW.Branch){
                    bool take=false;
                    if(IDEX.funct3=="000") take=(op1==op2);
                    else if(IDEX.funct3=="100") take =(op1<op2);
                    if(take){
                        branch_now=true;
                        branch_target =(size_t)(((long)IDEX.pc*4+IDEX.imm)/4);
                       
                    }
                }
                else if(IDEX.CW.Jump){
                    long ret_addr =(long)(IDEX.pc*4)+4;
                    nextEXMEM.aluOut=ret_addr;
                    nextEXMEM.write_pc4=true;
                    nextEXMEM.pc_plus4=ret_addr;
                  branch_now=true;
                        branch_target =(size_t)(((long)IDEX.pc*4+IDEX.imm)/4);
                }
                
                nextEXMEM.branch_taken = branch_now;
                nextEXMEM.target = branch_target;
            }
                if (IFID.valid) {
                    nextIDEX.valid=true;
                    nextIDEX.pc=IFID.pc;
                    
                    string inst=IFID.inst; 
                    string opcode=inst.substr(25,7),funct7=inst.substr(0,7),funct3=inst.substr(17,3);
                    nextIDEX.opcode=opcode; nextIDEX.funct7=funct7; nextIDEX.funct3= funct3;
                    nextIDEX.CW=maincontrol(opcode);
                    nextIDEX.rs1= (int)binToDec('0'+inst.substr(12,5));
                    nextIDEX.rs2= (int)binToDec('0'+inst.substr(7,5));
                    nextIDEX.rd= (int)binToDec('0'+inst.substr(20,5));
                    nextIDEX.val_rs1=binToDec(GPR[nextIDEX.rs1]);
                     nextIDEX.val_rs2=binToDec(GPR[nextIDEX.rs2]);
                     nextIDEX.imm= immGen(inst,opcode);
                }
                
                size_t nextPC = PC;
                
                if(EXMEM.valid && EXMEM.branch_taken){
                    nextIFID.valid=false;
                    nextIDEX.valid =false;
                    nextPC = EXMEM.target;
                }else{
                    if(PC < binaries.size()){
                        nextIFID.valid=true;
                        nextIFID.pc = PC;
                        nextIFID.inst = binaries[PC];
                        nextPC=PC+1;
                    
                    }
                }
                
                MEMWB = nextMEMWB; 
                EXMEM = nextEXMEM;
                IDEX = nextIDEX;
                IFID = nextIFID;
                PC = nextPC;
                
              
                
                GPR[0]=string(32,'0');
                
                printRegs(GPR);
                
            }
            cout<<"\n Final Registers:\n ";
                for(int i=0; i<32;i++) cout<<"x"<<i<<"="<<binToDec(GPR[i])<<"\n";
         }
    
    int main() {
        Assembler assembler;
        vector<string>binaries =assembler.assemble();
        assembler.singlecycleprocessor(binaries);
        fivestagepipeline(binaries);
        return 0;
    }
    
       
   
