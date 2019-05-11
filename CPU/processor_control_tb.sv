`timescale 1ns/1ns

module processor_control_tb;

	//Inputs
	reg[5:0] opcode;

	//Outputs
	wire RegDst;
    wire ALUSrc;
    wire MemtoReg;
    wire RegWrite;
    wire MemRead;
    wire MemWrite;
    wire Branch;
    wire BranchEqual;
    wire BranchNotEqual;
    wire[1:0] ALUOp;
    wire Jump;


    //Instantiation of Unit Under Test
        processor_control uut(
            .opcode(opcode),
            .RegDst(RegDst),
            .ALUSrc(ALUSrc),
            .MemtoReg(MemtoReg),
            .RegWrite(RegWrite),
            .MemRead(MemRead),
            .MemWrite(MemWrite),
            .Branch(Branch),
            .BranchEqual(BranchEqual),
            .BranchNotEqual(BranchNotEqual),
            .ALUOp(ALUOp),
            .Jump(Jump)
        );

	initial begin
	//Inputs initialization
		opcode = 6'b000000;
	end


    initial begin
	//Wait for the reset
		#100;
		 opcode = 6'b100000;
        #100;
		 opcode = 6'b100010;
		#100;
		 opcode = 6'b000000;
        #100;
		 opcode = 6'b000010;
        #100;
		 opcode = 6'b100100;
        #100;
		 opcode = 6'b100101;
        #100;
		 opcode = 6'b101010;
        #100;
		 opcode = 6'b100011;
        #100;
		 opcode = 6'b101011;
        #100;
		 opcode = 6'b000100;
        #100;
		 opcode = 6'b000101;
        #100;
		 opcode = 6'b000010;
	    $finish;
    end
	

endmodule