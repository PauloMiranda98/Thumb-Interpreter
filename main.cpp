#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>

#define VERSION 5
#define DEBUG 0

#define MODE_USR 0x10
#define MODE_FIQ 0x11
#define MODE_IRQ 0x12
#define MODE_SVC 0x13
#define MODE_ABT 0x17
#define MODE_UND 0x1B
#define MODE_SYS 0x1F

#define STATUS_N (1u<<31)
#define STATUS_Z (1u<<30)
#define STATUS_C (1u<<29)
#define STATUS_V (1u<<28)
#define STATUS_Q (1u<<27)

#define STATUS_J (1u<<24)

#define STATUS_I (1u<<7)
#define STATUS_F (1u<<6)
#define STATUS_T (1u<<5)

#define BITSET(x) ((1u<<x)-1)

#define NUMBER_REGISTER 16
#define SIZE_PROGRAM_MEMORY 256
#define BEGIN_PROGRAM_MEMORY 0x80000000
#define BEGIN_STACK_MEMORY 0x82000000

#define ADD 0
#define SUB 1

using namespace std;

typedef uint32_t word;
typedef uint32_t address;
typedef uint16_t word_tumb;
typedef uint8_t byte;

word cpsr;
word r[NUMBER_REGISTER];
map<address, byte> memory;

template <class T>
string printHex(T x){

    stringstream ss;
    ss << "0x" << hex << x;

    return ss.str();
}

char byteToChar(byte x){
    if(x <= 9)
        return char('0' + x);
    if(x <= 15)
        return char('A' + x - 10);

    return 'X';
}

string byteToString(byte x){
    string result;

    for(int i=4; i>=0; i-=4){
        result += byteToChar( (x>>i)&0xF );
    }

    return result;
}

string wordToString(word x){
    string result;

    for(int i=28; i>=0; i-=4){
        result += byteToChar( (x>>i)&0xF );
    }

    return result;
}

byte hexToDec(char x){
    if(x >= '0' and x <= '9')
        return byte(x-'0');

    if(x >= 'A' and x <= 'F')
        return byte(10 + (x-'A'));

    if(x >= 'a' and x <= 'f')
        return byte(10 + (x-'a'));

    return 0;
}

byte twoCharTobyte(char a, char b){
    return hexToDec(a)*16 + hexToDec(b);
}

word_tumb getInstructionThumb(address i){
    word_tumb result = 0;

    for(int j=0; j<2; j++){
        result += (memory[i+j] << (j*8) );
    }

    return result;
}
word getInstruction(address i){
    word result = 0;

    for(int j=0; j<4; j++){
        result += (memory[i+j] << (j*8) );
    }

    return result;
}


void writeMemory8(address i, byte x){
	if( (i>=BEGIN_PROGRAM_MEMORY) and ( (i+1)<=(BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) ) ){
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------DATA ABORT----------------------------------" << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << "Data Abort when accessing address " << printHex(i) << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << endl;
		return;
	}
		
    memory[i] = x;
}

void writeMemory16(address i, word_tumb x){
	if( (i>=BEGIN_PROGRAM_MEMORY) and ( (i+2)<=(BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) ) ){
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------DATA ABORT----------------------------------" << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << "Data Abort when accessing address " << printHex(i) << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << endl;
		return;
	}
	
    for(int j=0; j<2; j++){
        memory[i+j] = (x>>(j*8))&0xFF;
    }
}

void writeMemory32(address i, word x){
	if( (i>=BEGIN_PROGRAM_MEMORY) and ( (i+4)<=(BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) ) ){
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------DATA ABORT----------------------------------" << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << "Data Abort when accessing address " << printHex(i) << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << endl;
		return;
	}
	
    for(int j=0; j<4; j++){
        memory[i+j] = (x>>(j*8))&0xFF;
    }
}

byte readMemory8(address i){
	if( (i>=BEGIN_PROGRAM_MEMORY) and ( (i+1)<=(BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) ) ){
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------DATA ABORT----------------------------------" << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << "Data Abort when accessing address " << printHex(i) << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << endl;
		return 0;
	}
	
    return memory[i];
}

word_tumb readMemory16(address i){
	if( (i>=BEGIN_PROGRAM_MEMORY) and ( (i+2)<=(BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) ) ){
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------DATA ABORT----------------------------------" << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << "Data Abort when accessing address " << printHex(i) << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << endl;
		return 0;
	}
	
    word_tumb result = 0;

    for(int j=0; j<2; j++){
        result += (memory[i+j] << (j*8) );
    }

    return result;
}

word readMemory32(address i){
	if( (i>=BEGIN_PROGRAM_MEMORY) and ( (i+4)<=(BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) ) ){
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------DATA ABORT----------------------------------" << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << "Data Abort when accessing address " << printHex(i) << endl;
		cout << "---------------------------------------------------------------------" << endl;
		cout << endl;
		
		return 0;
	}
	
    word result = 0;

    for(int j=0; j<4; j++){
        result += (memory[i+j] << (j*8) );
    }

    return result;
}

void readProgramDebug(){
    string index, instruction;
    address i = BEGIN_PROGRAM_MEMORY;
    string s;
    while(getline(cin, s)){
        stringstream ss;
        ss << s;
        ss >> index >> instruction;

        for(int j=6; j>=0; j-=2)
            memory[i++] = twoCharTobyte(instruction[j], instruction[j+1]);
    }
}

void readProgram(){
    string index, instruction;
    address i = BEGIN_PROGRAM_MEMORY;

    while(cin >> index >> instruction){
        for(int j=6; j>=0; j-=2)
            memory[i++] = twoCharTobyte(instruction[j], instruction[j+1]);
    }
}

void showStatus(){
    cout << "---------------------------------------------------------------------" << endl;
    cout << "Registers" << endl;

    for(int i=0; i < 8; i++)
        cout << "R" << i << ": " << printHex(r[i]) << endl;

    cout << "SP" << ": " << printHex(r[13]) << endl;
    cout << "LR" << ": " << printHex(r[14]) << endl;
    cout << "PC" << ": " << printHex(r[15]) << endl;

    cout << "---------------------------------------------------------------------" << endl;
    cout << "STATUS" << endl;
    cout << "CPSR: ";
    cout << ( (cpsr&STATUS_N)!=0? 'N': 'n');
    cout << ( (cpsr&STATUS_Z)!=0? 'Z': 'z');
    cout << ( (cpsr&STATUS_C)!=0? 'C': 'c');
    cout << ( (cpsr&STATUS_V)!=0? 'V': 'v');
    cout << ( (cpsr&STATUS_Q)!=0? 'Q': 'q');
    cout << ( (cpsr&STATUS_J)!=0? 'J': 'j');
    cout << ( (cpsr&STATUS_I)!=0? 'I': 'i');
    cout << ( (cpsr&STATUS_F)!=0? 'F': 'f');
    cout << ( (cpsr&STATUS_T)!=0? 'T': 't');
    cout << '_';

    if( (cpsr&0x1F) == MODE_USR)
        cout << "SVC" << endl;
    else if( (cpsr&0x1F) == MODE_FIQ)
        cout << "FIQ" << endl;
    else if( (cpsr&0x1F) == MODE_IRQ)
        cout << "IRQ" << endl;
    else if( (cpsr&0x1F) == MODE_SVC)
        cout << "SVC" << endl;
    else if( (cpsr&0x1F) == MODE_ABT)
        cout << "ABT" << endl;
    else if( (cpsr&0x1F) == MODE_UND)
        cout << "UND" << endl;
    else if( (cpsr&0x1F) == MODE_SYS)
        cout << "SYS" << endl;

    cout << "---------------------------------------------------------------------" << endl;
    cout << "Program Memory Short" << endl;
    for(address i=BEGIN_PROGRAM_MEMORY; i < BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY; i+=4){
        cout << printHex(i) << ": " << printHex(wordToString(getInstruction(i))) << endl;
    }

/*
    cout << "---------------------------------------------------------------------" << endl;
    cout << "Program Memory Extended" << endl;
    for(address i=BEGIN_PROGRAM_MEMORY; i < BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY; i++){
        cout << printHex(i) << ": " << printHex(byteToString(readMemory8(i) )) << endl;
    }
*/
    cout << "---------------------------------------------------------------------" << endl;
    cout << "Data Memory Short" << endl;
    address shown = 0xFFFFFFFF;

    for(auto i: memory){
        address address = i.first - (i.first%4);

        if( (address >=BEGIN_PROGRAM_MEMORY) and (address < BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) )
            continue;
        if( (address >= r[13]) and (address < BEGIN_STACK_MEMORY) )
            continue;  
        if( shown == address)
            continue;

        cout << printHex(address) << ": " << printHex(wordToString(readMemory32(address))) << endl;
        shown = address;
    }

/*
    cout << "---------------------------------------------------------------------" << endl;
    cout << "Data Memory Extended" << endl;
    for(auto i: memory){
        address address = i.first;
        byte data = i.second;

        if( (address >=BEGIN_PROGRAM_MEMORY) and (address < BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY) )
            continue;
        if( (address >= r[13]) and (address < BEGIN_STACK_MEMORY) )
            continue;

        cout << printHex(address) << ": " << printHex(byteToString(data)) << endl;
    }
*/

    cout << "---------------------------------------------------------------------" << endl;
    cout << "Stack Memory Short" << endl;
    for(address i=r[13]; i < BEGIN_STACK_MEMORY; i+=4){
        cout << printHex(i) << ": " << printHex(wordToString(readMemory32(i))) << endl;
    }

/*
    cout << "---------------------------------------------------------------------" << endl;
    cout << "Stack Memory Extended" << endl;
    for(address i=BEGIN_PROGRAM_MEMORY; i < BEGIN_PROGRAM_MEMORY+SIZE_PROGRAM_MEMORY; i++){
        cout << printHex(i) << ": " << printHex(byteToString(readMemory8(i) )) << endl;
    }
*/

    cout << "---------------------------------------------------------------------" << endl;
    cout << endl << endl << endl;

}

void clearFlag(){
    word clear = STATUS_N | STATUS_Z | STATUS_V | STATUS_C | STATUS_Q;
    cpsr &= ~clear;
}

void updateN(word a){
    if(a&(1u<<31))
        cpsr |= STATUS_N;
    else
        cpsr &= ~STATUS_N;
}

void updateZ(word a){
    if(a == 0)
        cpsr |= STATUS_Z;
    else
        cpsr &= ~STATUS_Z;
}

void updateV(word a, word b, uint8_t op){
    if(op == ADD){
		int64_t overflow = int64_t( int32_t(a) ) + int64_t( int32_t(b) ); 
		
        if( (overflow > int64_t( numeric_limits<int32_t>::max() )) or (overflow < int64_t( numeric_limits<int32_t>::min() )))		
            cpsr |= STATUS_V;
        else
            cpsr &= ~STATUS_V;

    }else if(op == SUB){
		int64_t overflow = int64_t( int32_t(a) ) - int64_t( int32_t(b) ); 
		
        if( (overflow > int64_t( numeric_limits<int32_t>::max() )) or (overflow < int64_t( numeric_limits<int32_t>::min() )))		
            cpsr |= STATUS_V;
        else
            cpsr &= ~STATUS_V;
    }
}

void updateC(word a, word b, uint8_t op){
    if(op == ADD){
        if(a > (numeric_limits<uint32_t>::max() - b) )
            cpsr |= STATUS_C;
        else
            cpsr &= ~STATUS_C;

    }else if(op == SUB){
		if(b == 0)
            cpsr |= STATUS_C;
        else if(a > (numeric_limits<uint32_t>::max() - ( (~b) + 1)) )
            cpsr |= STATUS_C;
        else
            cpsr &= ~STATUS_C;
    }
}

bool decode(word_tumb instruction){

    if(instruction == 0x0)
		return false;
    if(instruction == 0xe7fe)
		return false;

    if( (instruction >> 6) == 0b0100000101){ //ADC
		
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
		#if DEBUG
			fprintf(stderr, "ADC R%d, R%d\n", int(rd), int(rm));
		#endif

        uint8_t carry_flag = 0;
        if(cpsr & STATUS_C)
            carry_flag = 1;
        else
            carry_flag = 0;

        r[rd] = r[rd] + r[rm] + word(carry_flag);

        updateN(r[rd]);
        updateZ(r[rd]);

        updateC(r[rm], carry_flag, ADD);
        if( (cpsr & STATUS_C) == 0)
            updateC(r[rd], r[rm] + carry_flag, ADD);

		int64_t overflow = int64_t( int32_t(r[rd]) ) + int64_t( int32_t(r[rm]) ) +  int64_t(carry_flag); 
		
        if( (overflow > int64_t( numeric_limits<int32_t>::max() )) or (overflow < int64_t( numeric_limits<int32_t>::min() )))
            cpsr |= STATUS_V;
        else
            cpsr &= ~STATUS_V;
				
        return true;
    }else if((instruction >> 9) == 0b0001110){ //ADD (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed3 = (instruction>>6) & BITSET(3);
		
		#if DEBUG
			fprintf(stderr, "ADD R%d, R%d, #%d\n", int(rd), int(rn), int(immed3));
		#endif

        r[rd] = r[rn] + word(immed3);

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(r[rn], immed3, ADD);
        updateV(r[rn], immed3, ADD);

        return true;
    }else if((instruction >> 11) == 0b00110){ //ADD (2)
        uint8_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);
		
		#if DEBUG
			fprintf(stderr, "ADD R%d, #%d\n", int(rd), int(immed8));
		#endif

        r[rd] = r[rd] + word(immed8);

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(r[rd], immed8, ADD);
        updateV(r[rd], immed8, ADD);

        return true;
    }else if((instruction >> 9) == 0b0001100){ //ADD (3)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "ADD R%d, R%d, R%d\n", int(rd), int(rn), int(rm));
		#endif

        r[rd] = r[rn] + r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(r[rn], r[rm], ADD);
        updateV(r[rn], r[rm], ADD);

        return true;
    }else if((instruction >> 8) == 0b01000100){ //ADD (4)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
        uint8_t h2 = (instruction>>6) & BITSET(1);
        uint8_t h1 = (instruction>>7) & BITSET(1);

		#if DEBUG
			fprintf(stderr, "ADD R%d, R%d\n", int(rd + 8*h1), int(rm + 8*h2));
		#endif


        if(h1 == 0 && h2 == 0){ // perguntar pro professor
            return true;
        }

        rd = rd + word(h1<<3); //add most significant bit
        rm = rm + word(h2<<3); //add most significant bit

        r[rd] = r[rd] + r[rm];
        //in this instruction don't need change the flag's

        return true;
    }else if((instruction >> 11) == 0b10100){ //ADD (5)
        uint16_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "ADD R%d, pc, #%d\n", int(rd), int(immed8*4));
		#endif

        r[rd] = (r[15] & 0xFFFFFFFC) + word(immed8 << 2); //r[15] = PC

        return true;
    }else if( (instruction >> 11) == 0b10101){ //ADD (6)
        uint16_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "ADD R%d, sp, #%d\n", int(rd), int(immed8*4));
		#endif

        r[rd] = r[13] + word(immed8 << 2); //r[13] = SP

        return true;
    }else if( (instruction >> 7) == 0b101100000){ //ADD (7)
        uint16_t immed7 = (instruction>>0) & BITSET(7);

		#if DEBUG
			fprintf(stderr, "ADD sp, #%d\n", int(immed7*4));
		#endif

        r[13] = r[13] + word(immed7 << 2); //r[13] = SP

        return true;
    }else if((instruction >> 6) == 0b0100000000){ // AND
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "AND R%d, R%d\n", int(rd), int(rm));
		#endif

        r[rd] = r[rd] & r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);

		return true;
    }else if((instruction >> 11) == 0b00010){ //ASR (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "ASR R%d, R%d, #%d\n", int(rd), int(rm), int(immed5));
		#endif

        if(immed5 == 0){
            if( (r[rm]>>31) & 1)
                cpsr |= (STATUS_C);
            else
                cpsr &= ~(STATUS_C);

            if( ((r[rm]>>31) & 1) == 0){
                r[rd] = 0;
            }else{
                r[rd] = 0xFFFFFFFF;
            }
        }else{
            if((r[rm]>>(immed5-1)) & 1){
                cpsr |= (STATUS_C );
            }else{
                cpsr &= ~(STATUS_C);
            }

            int32_t aux = int32_t(r[rm]);
            aux = aux>>immed5;
            r[rd] = word(aux);
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if((instruction >> 6) == 0b0100000100){ //ASR (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rs = (instruction>>3) & BITSET(3);
        uint8_t rs_7_0 = (r[rs]&BITSET(8));

		#if DEBUG
			fprintf(stderr, "ASR R%d, R%d\n", int(rd), int(rs) );
		#endif

        if( rs_7_0 == 0 ){
            //Fazer nada
        }else if( rs_7_0 < 32 ){
            if( r[rd] & (1u<<(rs_7_0 - 1)) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            int32_t aux = int32_t(r[rd]);
            aux = (aux>>rs_7_0);
            r[rd] = word(aux);
        }else if(rs_7_0 >= 32){
            if( r[rd] & (1u<<31) ){
                cpsr |= STATUS_C;
                r[rd] = 0xFFFFFFFF;
            }else{
                cpsr &= ~STATUS_C;
                r[rd] = 0;
            }
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( ((instruction>>12) == 0b1101) && ( ((instruction>>8)&0xF) != 0b1111) ){ //B
        int8_t signed_immed_8 = (instruction>>0) & BITSET(8);
        uint8_t cond = (instruction>>8) & BITSET(4);

        switch(cond){
            case 0: //EQ
				#if DEBUG
					fprintf(stderr, "BEQ 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_Z) != 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                }

				return true;
            break;
            case 1: //NE
				#if DEBUG
					fprintf(stderr, "BNE 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_Z) == 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                }
				return true;
            break;
            case 2: //CS/HS
				#if DEBUG
					fprintf(stderr, "BHS 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_C) != 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 3://CC/LO
				#if DEBUG
					fprintf(stderr, "BLO 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_C) == 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 4://MI
				#if DEBUG
					fprintf(stderr, "BMI 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_N) != 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 5://PL
				#if DEBUG
					fprintf(stderr, "BPL 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_N) == 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 6://VS
				#if DEBUG
					fprintf(stderr, "BVS 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_V) != 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 7://VC
				#if DEBUG
					fprintf(stderr, "BVC 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if((cpsr & STATUS_V) == 0){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 8: //HI
				#if DEBUG
					fprintf(stderr, "BHI 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if(( (cpsr & STATUS_Z) == 0) && ((cpsr & STATUS_C) != 0)){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 9: //LS
				#if DEBUG
					fprintf(stderr, "BLS 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if(((cpsr & STATUS_C) == 0) || ((cpsr & STATUS_Z) != 0)){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 10: //GE
				#if DEBUG
					fprintf(stderr, "BGE 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif				
                if( ((cpsr>>28) & 1) == ((cpsr>>31)&1) ){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 11: //LT
				#if DEBUG
					fprintf(stderr, "BLT 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if( ( (cpsr>>28) & 1) != ((cpsr>>31)&1) ){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);                    
                    return true;
                }
            break;
            case 12://GT
				#if DEBUG
					fprintf(stderr, "BGT 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if(( (cpsr & STATUS_Z) == 0) && (( (cpsr>>28) & 1) == ((cpsr>>31)&1) ) ){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 13://LE
				#if DEBUG
					fprintf(stderr, "BLE 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
                if(((cpsr & STATUS_Z) != 0) || (((cpsr>>28) & 1) != ((cpsr>>31)&1)) ){
                    r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
                    return true;
                }
            break;
            case 14://AL
				#if DEBUG
					fprintf(stderr, "BAL 0x%x\n", r[15] + 2 + word(int32_t(signed_immed_8) << 1) );
				#endif
				
				r[15] = r[15] + 2 + word(int32_t(signed_immed_8) << 1);
				return true;
            break;
          
        }
        return true;
    }else if((instruction>>11) == 0b11100){ // B (2)
        int32_t signed_immed11 = (instruction>>0) & BITSET(11);
        
        for(int i = 11; i <= 31;i++){
			signed_immed11 |= ((signed_immed11 >> 10) & 1) << i;
        }

        r[15] = r[15] + 2 + (signed_immed11 << 1);

		#if DEBUG
			fprintf(stderr, "B 0x%x\n", r[15]);
		#endif

        return true;
    }else if((instruction>>6) == 0b0100001110){ // BIC
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "BIC R%d, R%d\n", int(rd), int(rm));
		#endif

        r[rd] = r[rd] & (~r[rm]);

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if((instruction>>8) == 0b10111110){ //BKPT
		cout << "---------------------------------------------------------------------" << endl;
		cout << "-------------------------BREAKPOINT----------------------------------" << endl;
		showStatus();
		cout << "---------------------------------------------------------------------" << endl;

        return true;
    }else if( (instruction>>13) == 0b111){ //BLX(1)
		uint32_t offset_11 = (instruction>>0) & BITSET(11);
		uint8_t H = (instruction>>11) & BITSET(2);
                
		if(H == 0b10){
			offset_11 <<= 12;
			for(int i = 23; i <= 31;i++){
				offset_11 |= ((offset_11 >> 22) & 1) << i;
			}
			
			r[14] = r[15] + (int32_t(offset_11));
			
			#if DEBUG
				fprintf(stderr, "BL **(Save LR)\n");
			#endif
		}else if(H == 0b11){
			word temp = r[15];
			r[15] = r[14] + 2 + (offset_11<<1);
			r[14] = temp | 1; 
			
			#if DEBUG
				fprintf(stderr, "BL 0x%x\n", r[15]);
			#endif

		}else if(H == 0b01){
			word temp = r[15];
			r[15] = (r[14] + (offset_11 << 1)) & ~1;
			r[14] = temp | 1;
			
			cpsr &= ~STATUS_T;
			
			#if DEBUG
				fprintf(stderr, "BLX 0x%x\n", r[15]);
			#endif
		}
		return true;
	}else if( (instruction>>7) == 0b010001111){ //BLX (2)
		uint8_t rm =  (instruction>>3) & BITSET(3);
		uint8_t h2 =  (instruction>>6) & 1;

		rm |= (h2<<3);
		r[14] = (r[15] | 1);
		
		if(r[rm] & 1)
			cpsr |= STATUS_T;
        else
            cpsr &= ~STATUS_T;
            
        r[15] = (r[rm] & 0xFFFFFFFE);
        
		#if DEBUG
			fprintf(stderr, "BLX 0x%x\n", r[15]);
		#endif
		
		return true;	
	}else if((instruction>>7) == 0b010001110){ //BX
		uint8_t rm =  (instruction>>3) & BITSET(3);
		uint8_t h2 =  (instruction>>6) & 1;

		rm |= (h2<<3);
		if(r[rm] & 1)
			cpsr |= STATUS_T;
        else
            cpsr &= ~STATUS_T;
            
        r[15] = (r[rm] & 0xFFFFFFFE);
        
		#if DEBUG
			fprintf(stderr, "BX 0x%x\n", r[15]);
		#endif
		
        return true ;
	}else if( (instruction>>6) == 0b0100001011){ // CMN
        uint8_t rn = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "CMN R%d, R%d\n", int(rn), int(rm) );
		#endif

        word alu_out = r[rn] + r[rm];

        updateN(alu_out);
        updateZ(alu_out);
        updateC(r[rn], r[rm], ADD);
        updateV(r[rn], r[rm], ADD);

        return true;
    }else if( (instruction>>11) == 0b00101){ // CMP (1)
        uint8_t immed_8 = (instruction>>0) & BITSET(8);
        uint8_t rn = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "CMP R%d, #%d\n", int(rn), int(immed_8));
		#endif

        word alu_out = r[rn] - immed_8;

        updateN(alu_out);
        updateZ(alu_out);
        updateC(r[rn], immed_8, SUB);
        updateV(r[rn], immed_8, SUB);

        return true;
    }else if( (instruction>>6) == 0b0100001010){ // CMP (2)
        uint8_t rn = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "CMP R%d, R%d\n", int(rn), int(rm) );
		#endif

        word alu_out = r[rn] - r[rm];

        updateN(alu_out);
        updateZ(alu_out);
        updateC(r[rn], r[rm], SUB);
        updateV(r[rn], r[rm], SUB);

        return true;
    }else if( (instruction>>8) == 0b01000101){ // CMP (3)
        uint8_t rn = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
        uint8_t h2 = (instruction>>6) & BITSET(1);
        uint8_t h1 = (instruction>>7) & BITSET(1);

		#if DEBUG
			fprintf(stderr, "CMP R%d, R%d\n", int(8*h1 + rn), int(8*h2 + rm) );
		#endif

        if( (h1==0) and (h2==0) ) //UNPREDICTABLE
            return true;

        word alu_out = r[8*h1 + rn] - r[8*h2 + rm];

        updateN(alu_out);
        updateZ(alu_out);
        updateC(r[8*h1 + rn], r[8*h2 + rm], SUB);
        updateV(r[8*h1 + rn], r[8*h2 + rm], SUB);

        return true;
    }else if( (instruction>>6) == 0b0100000001){ // EOR
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "EOR R%d, R%d\n", int(rd), int(rm));
		#endif

        r[rd] = r[rd] ^ r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( (instruction>>11) == 0b11001){ // LDMIA
       uint8_t register_list = (instruction>>0) & BITSET(8);
       uint8_t rn = (instruction>>8) & BITSET(3);

       u_int8_t num_set_bit_register = 0;

       for(int i=0; i<8; i++)
           if( register_list & (1<<i) )
               num_set_bit_register++;

       address start_address = r[rn];
       //address end_address = r[rn] + (4*num_set_bit_register) - 4;
       address addr = start_address;

		#if DEBUG
			fprintf(stderr, "LDMIA R%d!, {", int(rn));
		#endif

       for(int i=0; i<8; i++){
           if( register_list & (1<<i) ){
               r[i] = readMemory32(addr);
               addr = addr + 4;
               
				#if DEBUG
					fprintf(stderr, "R%d ", i);
				#endif

           }
       }
		#if DEBUG
			fprintf(stderr, "}\n");
		#endif

        //if(addr == end_address) OK
        r[rn] = r[rn] + (4*num_set_bit_register);

       return true;
   }else if((instruction>>11) == 0b01101){ // LDR (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "lDR R%d, [R%d, #%d]\n", int(rd), int(rn), int(immed5*4));
		#endif

        address addr = r[rn] + (immed5*4);

        word data = 0;
        if( (addr & BITSET(2)) == 0){
            data = readMemory32(addr);
        }else{
            //data = UNPRECTABLE;
        }

        r[rd] = data;

        return true;
    }else if((instruction>>9) == 0b0101100){ // LDR (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDR R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm));
		#endif

        address addr = r[rn] + r[rm];
        word data = 0;
        if( (addr & BITSET(2)) == 0){
            data = readMemory32(addr);
        }else{
            //data = UNPRECTABLE;
        }

        r[rd] = data;

        return true;
    }else if((instruction>>11) == 0b01001){ // LDR (3)
        uint16_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDR R%d, [PC, #%d]\n", int(rd), int(immed8*4));
		#endif

        address addr = (r[15]&0xfffffffc) + (immed8*4);

        r[rd] = readMemory32(addr);

        return true;
    }else if((instruction>>11) == 0b10011){ // LDR (4)
        uint16_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDR R%d, [SP, #%d]\n", int(rd), int(immed8*4));
		#endif

        address addr = r[13] + (immed8*4);

        word data = 0;
        if( (addr & BITSET(2)) == 0){
            data = readMemory32(addr);
        }else{
            //data = UNPRECTABLE;
        }

        r[rd] = data;

        return true;
    }else if((instruction>>11) == 0b01111){ // LDRB (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "lDRB R%d, [R%d, #%d]\n", int(rd), int(rn), int(immed5) );
		#endif

        address addr = r[rn] + (immed5);

        r[rd] = readMemory8(addr);

        return true;
    }else if((instruction>>9) == 0b0101110){ // LDRB (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDRB R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif

        address addr = r[rn] + r[rm];

        r[rd] = readMemory8(addr);

        return true;
    }else if((instruction>>11) == 0b10001){ // LDRH (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "lDRH R%d, [R%d, #%d]\n", int(rd), int(rn), int(immed5*2) );
		#endif

        address addr = r[rn] + (immed5*2);

        word data = 0;
        if( (addr&1) == 0){
            data = readMemory16(addr);
        }else{
            //data = UNPRECTABLE;
        }

        r[rd] = data;

        return true;
    }else if((instruction>>9) == 0b0101101){ // LDRH (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDRH R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif

        address addr = r[rn] + r[rm];
        word data = 0;
        if( (addr&1) == 0){
            data = readMemory16(addr);
        }else{
            //data = UNPRECTABLE;
        }

        r[rd] = data;

        return true;
    }else if((instruction>>9) == 0b0101011){ // LDRSB
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDRSB R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif

        address addr = r[rn] + r[rm];

        int8_t data = 0;
		data = int8_t(readMemory8(addr));

        r[rd] = word(int32_t(data));

        return true;
    }else if((instruction>>9) == 0b0101111){ // LDRSH
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "lDRSH R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif

        address addr = r[rn] + r[rm];

        int16_t data = 0;
        if( (addr&1) == 0){
            data = int16_t(readMemory16(addr));
        }else{
            //data = UNPRECTABLE;
        }

        r[rd] = word(int32_t(data));

        return true;
    }else if((instruction>>11) == 0b00000){ // LSL (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "LSL R%d, R%d, #%d\n", int(rd), int(rm), int(immed5) );
		#endif

        if(immed5 > 0){
            if( r[rm] & (1u<<(32-immed5)) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = (r[rm]<<immed5);
        }else{
            r[rd] = r[rm];
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if((instruction>>6) == 0b0100000010){ // LSL (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rs = (instruction>>3) & BITSET(3);
        uint8_t rs_7_0 = (r[rs]&BITSET(8));

		#if DEBUG
			fprintf(stderr, "LSL R%d, R%d\n", int(rd), int(rs) );
		#endif

        if( rs_7_0 == 0 ){
            //Fazer nada
        }else if( rs_7_0 < 32 ){
            if( r[rd] & (1u<<(32 - rs_7_0)) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = (r[rd]<<rs_7_0);
        }else if( rs_7_0 == 32 ){
            if( r[rd] & 1)
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = 0;
        }else{
            cpsr &= ~STATUS_C;
            r[rd] = 0;
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if((instruction>>11) == 0b00001){ // LSR (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "LSR R%d, R%d, #%d\n", int(rd), int(rm), int(immed5) );
		#endif

        if(immed5 > 0){
            if( r[rm] & (1u<<(immed5-1)) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = (r[rm]>>immed5);
        }else{
            if( r[rm] & (1u<<31) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = 0;
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( (instruction>>6) == 0b0100000011){ // LSR (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rs = (instruction>>3) & BITSET(3);
        uint8_t rs_7_0 = (r[rs]&BITSET(8));

		#if DEBUG
			fprintf(stderr, "LSR R%d, R%d\n", int(rd), int(rs));
		#endif

        if( rs_7_0 == 0 ){
            //Fazer nada
        }else if( rs_7_0 < 32 ){
            if( r[rd] & (1u<<(rs_7_0 - 1)) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = (r[rd]>>rs_7_0);
        }else if( rs_7_0 == 32 ){
            if( r[rd] & (1u<<31) )
                cpsr |= STATUS_C;
            else
                cpsr &= ~STATUS_C;

            r[rd] = 0;
        }else{
            cpsr &= ~STATUS_C;
            r[rd] = 0;
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if((instruction>>11) == 0b00100){ // MOV (1)
        uint8_t immed_8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "MOV R%d, #%d\n", int(rd), int(immed_8) );
		#endif

        r[rd] = word(immed_8);

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( (instruction>>6) == 0b0001110000){ // MOV (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "MOV R%d, R%d\n", int(rd), int(rn) );
		#endif

        r[rd] = r[rn];

        updateN(r[rd]);
        updateZ(r[rd]);
        cpsr &= ~STATUS_C;
        cpsr &= ~STATUS_V;

        return true;
    }else if( (instruction>>8) == 0b01000110){ // MOV (3)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);
        uint8_t h2 = (instruction>>6) & BITSET(1);
        uint8_t h1 = (instruction>>7) & BITSET(1);

        if( (h1==0) and (h2==0) ) //UNPREDICTABLE
            return true;

		#if DEBUG
			fprintf(stderr, "MOV R%d, R%d\n", int(h1*8 + rd), int(h2*8 + rm) );
		#endif

        r[h1*8 + rd] = r[h2*8 + rm];

        return true;
    }else if( (instruction>>6) == 0b0100001101){ // MUL
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "MUL R%d, R%d\n", int(rd), int(rm) );
		#endif

        r[rd] = word(r[rd] * r[rm]);

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( (instruction>>6) == 0b0100001111){ // MVN
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "MVN R%d, R%d\n", int(rd), int(rm) );
		#endif

        r[rd] = ~r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( (instruction>>6) == 0b0100001001){ // NEG
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "NEG R%d, R%d\n", int(rd), int(rm) );
		#endif

        r[rd] = word(0) - r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(0, r[rd], SUB);
        updateV(0, r[rd], SUB);

        return true;
    }else if( (instruction>>6) == 0b0100001100){ // ORR
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "ORR R%d, R%d\n", int(rd), int(rm) );
		#endif

        r[rd] = r[rd] | r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if((instruction>>9) == 0b1011110){ // POP
       uint8_t register_list = (instruction>>0) & BITSET(8);
       uint8_t R = (instruction>>8) & BITSET(1);

       u_int8_t num_set_bit_register = 0;
       for(int i=0; i<8; i++)
           if( register_list & (1<<i) )
               num_set_bit_register++;

       address start_address = r[13];
       address end_address = r[13] + 4*(R + num_set_bit_register);
       address addr = start_address;

		#if DEBUG
			fprintf(stderr, "POP { ");
		#endif

       for(int i=0; i<8; i++){
           if( register_list & (1<<i) ){
               r[i] = readMemory32(addr);
               addr = addr + 4;
               
				#if DEBUG
					fprintf(stderr, "R%d ", i);
				#endif
           }
       }
              
        if(R == 1){
			#if DEBUG
				fprintf(stderr, "PC ");
			#endif
           word value = readMemory32(addr);
           r[15] = value & 0xfffffffe;

           if(VERSION >= 5){
               if(value&1)
                   cpsr |= STATUS_T;
               else
                   cpsr &= ~STATUS_T;
            }

            addr = addr + 4;
        }
        
		#if DEBUG
			fprintf(stderr, "}\n ");
		#endif

        //if(addr == end_address) OK
        r[13] = end_address;

       return true;
   }else if((instruction>>9) == 0b1011010){ // PUSH
       uint8_t register_list = (instruction>>0) & BITSET(8);
       uint8_t R = (instruction>>8) & BITSET(1);

       u_int8_t num_set_bit_register = 0;
       for(int i=0; i<8; i++)
           if( register_list & (1<<i) )
               num_set_bit_register++;

       address start_address = r[13] - 4*(R + num_set_bit_register);
       //address end_address = r[13] - 4;
       address addr = start_address;
              
		#if DEBUG
			fprintf(stderr, "PUSH { ");
		#endif

       for(int i=0; i<8; i++){
           if( register_list & (1<<i) ){
               writeMemory32(addr, r[i]);
               addr = addr + 4;
				#if DEBUG
					fprintf(stderr, "R%d ", i);
				#endif
           }
       }
       

        if(R == 1){
			#if DEBUG
				fprintf(stderr, "LR ");
			#endif
           writeMemory32(addr, r[14]);
            addr = addr + 4;
        }
        
		#if DEBUG
			fprintf(stderr, "}\n");
		#endif

        //if(addr-4 == end_address) OK
        r[13] = r[13] - 4*(R + num_set_bit_register);

       return true;
   }else if( (instruction>>6) == 0b0100000111){ // ROR
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rs = (instruction>>3) & BITSET(3);
        uint8_t rs_7_0 = (r[rs]&BITSET(8));

		#if DEBUG
			fprintf(stderr, "ROR R%d, R%d\n", int(rd), int(rs) );
		#endif

        if( rs_7_0 == 0 ){
            //Fazer nada
        }else{
            uint8_t rs_4_0 = (r[rs]&BITSET(5));
            if( rs_4_0 == 0 ){
                if( r[rd] & (1u<<31) )
                    cpsr |= STATUS_C;
                else
                    cpsr &= ~STATUS_C;
            }else{
                if( r[rd] & (1u<<(rs_4_0 - 1)) )
                    cpsr |= STATUS_C;
                else
                    cpsr &= ~STATUS_C;

                r[rd] = ((r[rd] & ((1 << rs_4_0) - 1)) << (32 - rs_4_0)) | (r[rd] >> rs_4_0);
            }
        }

        updateN(r[rd]);
        updateZ(r[rd]);

        return true;
    }else if( (instruction>>6) == 0b0100000110){ // SBC
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "SBC R%d, R%d\n", int(rd), int(rm) );
		#endif

        word not_carry = (cpsr&STATUS_C)? 0: 1;

        r[rd] = r[rd] - r[rm] - not_carry;

        updateN(r[rd]);
        updateZ(r[rd]);
        
        updateC(r[rd], r[rm], SUB);        
        if( (cpsr & STATUS_C) == 0)
            updateC(r[rd]-r[rm], not_carry, SUB);

		int64_t overflow = int64_t( int32_t(r[rd]) ) - int64_t( int32_t(r[rm]) ) -  int64_t(not_carry); 
		
        if( (overflow > int64_t( numeric_limits<int32_t>::max() )) or (overflow < int64_t( numeric_limits<int32_t>::min() )))
            cpsr |= STATUS_V;
        else
            cpsr &= ~STATUS_V;

        return true;
    }else if((instruction>>11) == 0b11000){ // STMIA
       uint8_t register_list = (instruction>>0) & BITSET(8);
       uint8_t rn = (instruction>>8) & BITSET(3);

       u_int8_t num_set_bit_register = 0;
       for(int i=0; i<8; i++)
           if( register_list & (1<<i) )
               num_set_bit_register++;

       address start_address = r[rn];
       //address end_address = r[rn] + (4*num_set_bit_register) - 4;
       address addr = start_address;

		#if DEBUG
			fprintf(stderr, "STMIA R%d!, { ", int(rn));
		#endif

       for(int i=0; i<8; i++){
           if( register_list & (1<<i) ){
               writeMemory32(addr, r[i]);
               addr = addr + 4;
				#if DEBUG
					fprintf(stderr, "R%d ", i);
				#endif
           }
       }
		#if DEBUG
			fprintf(stderr, "}\n");
		#endif

        //if(addr-4 == end_address) OK
        r[rn] = r[rn] + (4*num_set_bit_register);

       return true;
   }else if((instruction>>11) == 0b01100){ // STR (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "STR R%d, [R%d, #%d]\n", int(rd), int(rn), int(immed5*4) );
		#endif

        address addr = r[rn] + (immed5*4);

        if( (addr & BITSET(2)) == 0x0){
            writeMemory32(addr, r[rd]);
        }else{
            //writeMemory32(UNPREDICTABLE);
        }

        return true;
    }else if( (instruction>>9) == 0b0101000){ // STR (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "STR R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif

        address addr = r[rm] + r[rn];

        if( (addr & BITSET(2)) == 0x0){
            writeMemory32(addr, r[rd]);
        }else{
            //writeMemory32(UNPREDICTABLE);
        }

        return true;
    }else if( (instruction>>11) == 0b10010){ // STR (3)
        uint16_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "STR R%d, [SP, #%d]\n", int(rd), int(immed8*4) );
		#endif

        address addr = r[13] + (immed8*4);

        if( (addr & BITSET(2)) == 0x0){
            writeMemory32(addr, r[rd]);
        }else{
            //writeMemory32(UNPREDICTABLE);
        }

        return true;
    }else if((instruction>>11) == 0b01110){ // STRB (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "STRB R%d, [R%d, #%d]\n", int(rd), int(rn), int(immed5) );
		#endif

        address addr = r[rn] + immed5;

        writeMemory8(addr, r[rd]&BITSET(8));

        return true;
    }else if( (instruction>>9) == 0b0101010){ // STRB (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "STRB R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif
		
        address addr = r[rn] + r[rm];

        writeMemory8(addr, r[rd]&BITSET(8));

        return true;
    }else if((instruction>>11) == 0b10000){ // STRH (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed5 = (instruction>>6) & BITSET(5);

		#if DEBUG
			fprintf(stderr, "STRH R%d, [R%d, #%d]\n", int(rd), int(rn), int(immed5*2) );
		#endif

        address addr = r[rn] + (immed5*2);

        if( (addr & BITSET(2)) == 0x0){ //Conferir
            writeMemory16(addr, r[rd]&BITSET(16));
        }else{
            //writeMemory16(UNPREDICTABLE);
        }

        return true;
    }else if( (instruction>>9) == 0b0101001){ // STRH (2)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "STRH R%d, [R%d, R%d]\n", int(rd), int(rn), int(rm) );
		#endif

        address addr = r[rn] + r[rm];

        if( (addr & BITSET(2)) == 0x0){//Conferir
            writeMemory16(addr, r[rd]&BITSET(16));
        }else{
            //writeMemory16(UNPREDICTABLE);
        }

        return true;
    }else if((instruction>>9) == 0b0001111){ // SUB (1)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t immed3 = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "SUB R%d, R%d, #%d\n", int(rd), int(rn), int(immed3) );
		#endif

        r[rd] = r[rn] - immed3;

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(r[rn], immed3, SUB);
        updateV(r[rn], immed3, SUB);

        return true;
    }else if((instruction>>11) == 0b00111){ // SUB (2)
        uint8_t immed8 = (instruction>>0) & BITSET(8);
        uint8_t rd = (instruction>>8) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "SUB R%d, #%d\n", int(rd), int(immed8) );
		#endif

        r[rd] = r[rd] - immed8;

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(r[rd], immed8, SUB);
        updateV(r[rd], immed8, SUB);

        return true;
    }else if((instruction>>9) == 0b0001101){ // SUB (3)
        uint8_t rd = (instruction>>0) & BITSET(3);
        uint8_t rn = (instruction>>3) & BITSET(3);
        uint8_t rm = (instruction>>6) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "SUB R%d, R%d, R%d\n", int(rd), int(rn), int(rm) );
		#endif

        r[rd] = r[rn] - r[rm];

        updateN(r[rd]);
        updateZ(r[rd]);
        updateC(r[rn], r[rm], SUB);
        updateV(r[rn], r[rm], SUB);

        return true;
    }else if((instruction>>7) == 0b101100001){ // SUB (4)
        uint16_t immed7 = (instruction>>0) & BITSET(7);

		#if DEBUG
			fprintf(stderr, "SUB SP, #%d\n", int(immed7*4) );
		#endif

        r[13] = r[13] - word(immed7<<2);

        return true;
    }else if( (instruction>>8) == 0b11011111){ // SWI
        uint8_t immed8 = (instruction>>0) & BITSET(8);

        if( (cpsr & STATUS_I) == 0 ){ //Conferir com professor
			cout << "---------------------------------------------------------------------" << endl;
			cout << "--------------------------------SWI----------------------------------" << endl;
 			cout << "---------------------------------------------------------------------" << endl;
            cout << "SWI interrupt number is " << printHex( int(immed8) ) << endl;
			cout << "---------------------------------------------------------------------" << endl;
			cout << endl;
        }

        return true;
    }else if( (instruction>>6) == 0b0100001000){ // TST
        uint8_t rn = (instruction>>0) & BITSET(3);
        uint8_t rm = (instruction>>3) & BITSET(3);

		#if DEBUG
			fprintf(stderr, "TST R%d, R%d\n", int(rn), int(rm) );
		#endif

        word alu_out = r[rn] & r[rm];

        updateN(alu_out);
        updateZ(alu_out);

        return true;
    }	

    return false;
}

void run(){
    cpsr = 0;
    cpsr |= MODE_SVC;
    cpsr |= STATUS_T;

    r[13] = BEGIN_STACK_MEMORY;
    r[14] = BEGIN_PROGRAM_MEMORY; //ou O
    r[15] = BEGIN_PROGRAM_MEMORY;

    while(1){		
        word_tumb instruction = getInstructionThumb(r[15]);

        r[15] += 2;

        if(decode(instruction) == false)
            break;

    }

}

int main(){
//    readProgramDebug();
    readProgram();
    run();
    
    cout << "---------------------------------------------------------------------" << endl;
    cout << "------------------------SHOW STATUS----------------------------------" << endl;
    showStatus();

    return 0;
}
