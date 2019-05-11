`timescale 1ns/1ns

module alu_controlUnit_tb;

	//Inputs
	reg [1: 0] ALUOp;
	reg [5: 0] opcode;


	//Outputs
	wire [2:0] alu_control;


	//Instantiation of Unit Under Test
	alu_controlUnit uut (
		.ALUOp(ALUOp),
		.opcode(opcode),
		.alu_control(alu_control)
	);


	initial begin
	//Inputs initialization
		ALUOp = 0;
		opcode = 0;
	end


	initial begin
	//Wait for the reset
		#100;
			ALUOp = 00;
			opcode = 100000;
		#100;
			ALUOp = 00;
			opcode = 100010;
		#100;
			ALUOp = 00;
			opcode = 000000;
		#100;
			ALUOp = 00;
			opcode = 000010;
		#100;
			ALUOp = 00;
			opcode = 100100;
		#100;
			ALUOp = 00;
			opcode = 100101;
		#100;
			ALUOp = 00;
			opcode = 101010;
		#100;
			ALUOp = 10;
			opcode = 100011;
		#100;
			ALUOp = 10;
			opcode = 101011;
		#100;
			ALUOp = 01;
			opcode = 000100;
		#100;
			ALUOp = 01;
			opcode = 000101;
		$finish;
	end

endmodule