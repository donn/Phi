`timescale 1ns/1ns

module alu_tb;

	//Inputs
	reg [31:0] data_1;
	reg [31:0] data_2;
	reg [2:0] alu_control;


	//Outputs
	wire [31:0] result;
	wire zero_flag;


	//Instantiation of Unit Under Test
	alu uut (
		.data_1(data_1),
		.data_2(data_2),
		.alu_control(alu_control),
		.result(result),
		.zero_flag(zero_flag)
	);


	initial begin
	//Inputs initialization
		data_1 = 0;
		data_2 = 0;
		alu_control = 0;
	end


	initial begin
	//Wait for the reset
		#100;
			data_1 = 1;
			data_2 = 2;
			alu_control = 3'b000;
		#100;
			data_1 = 4;
			data_2 = 2;
			alu_control = 3'b001;
		#100;
			data_1 = 3;
			data_2 = 0;
			alu_control = 3'b010;
		#100;
			data_1 = 1;
			data_2 = 2;
			alu_control = 3'b011;
		#100;
			data_1 = 1;
			data_2 = 2;
			alu_control = 3'b100;
		#100;
			data_1 = 3;
			data_2 = 3;
			alu_control = 3'b101;
		#100;
			data_1 = 3;
			data_2 = 3;
			alu_control = 3'b110;
		#100;
			data_1 = 1;
			data_2 = 2;
			alu_control = 3'b111;
		$finish;

	end

endmodule