`timescale 1ns/1ns

module datapath_tb;

	//Inputs
	reg clk;
    reg rst;
    reg RegDst;
    reg ALUSrc;
    reg MemtoReg;
    reg RegWrite;
    reg MemRead;
    reg MemWrite;
    reg Branch;
    reg BranchEqual;
    reg BranchNotEqual;
    reg[1:0] ALUOp;
    reg Jump;

	//Outputs
	wire[5:0] opcode;


    //Instantiation of Unit Under Test
        datapath uut(
            .clk(clk),
            .rst(rst),
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
            .Jump(Jump),
            .opcode(opcode)
        );

	initial begin
	//Inputs initialization
        clk = 0;
        rst = 1;
        RegDst = 0;
        ALUSrc = 0;
        MemtoReg = 0;
        RegWrite = 0;
        MemRead = 0;
        MemWrite = 0;
        Branch = 0;
        BranchEqual = 0;
        BranchNotEqual = 0;
        ALUOp = 0;
        Jump = 0;
	end

    always 
    #5  clk =  ~clk; 


    initial begin
	//Wait for the reset
		#100;
            rst = 1;
            RegDst = 0;
            ALUSrc = 0;
            MemtoReg = 0;
            RegWrite = 0;
            MemRead = 0;
            MemWrite = 0;
            Branch = 0;
            BranchEqual = 0;
            BranchNotEqual = 0;
            ALUOp = 0;
            Jump = 0;
	    $finish;
    end
	

endmodule