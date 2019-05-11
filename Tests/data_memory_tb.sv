`timescale 1ns/1ns

module data_memory_tb;

	//Inputs
	reg clk;
	reg [31: 0] address;
	reg MemWrite;
	reg MemRead;
	reg [31: 0] write_data;


	//Outputs
	wire [31: 0] read_data;


	//Instantiation of Unit Under Test
	data_memory uut (
		.clk(clk),
		.address(address),
		.MemWrite(MemWrite),
		.MemRead(MemRead),
		.write_data(write_data),
		.read_data(read_data)
	);


	initial begin
	//Inputs initialization
		clk = 0;
		address = 0;
		MemWrite = 0;
		MemRead = 0;
		write_data = 0;
	end
	
	always 
    #5  clk =  ~clk; 


    initial begin
	//Wait for the reset
		#100;
			address = 5;
			MemWrite = 0;
			MemRead = 1;
			write_data = 0;
		#100;
			address = 10;
			MemWrite = 1;
			MemRead = 0;
			write_data = 13;
	    $finish;
    end

endmodule