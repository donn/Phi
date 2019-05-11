`timescale 1ns/1ns

module register_file_tb;

	//Inputs
	reg clk;
    reg rst;
    reg RegWrite;
    reg[4:0] write_address;
    reg[31:0] write_data;
    reg[4:0] read_address_1;
    reg[4:0] read_address_2;


	//Outputs
	wire[31:0] read_data_1;
    wire[31:0] read_data_2;


	//Instantiation of Unit Under Test
    register_file uut(
        .clk(clk),
        .rst(rst),
        .RegWrite(RegWrite),
        .write_address(write_address),
        .write_data(write_data),
        .read_address_1(read_address_1),
        .read_address_2(read_address_2),
        .read_data_1(read_data_1),
        .read_data_2(read_data_2)
    );


	initial begin
	//Inputs initialization
		 clk = 0;
         rst = 1;
         RegWrite = 0;
         write_address = 0;
         write_data = 0;
         read_address_1 = 0;
         read_address_2 = 0;
	end
	
	always 
    #5  clk =  ~clk; 


    initial begin
	//Wait for the reset
		#100;
            rst = 0;
            RegWrite = 1;
            write_address = 4;
            write_data = 22;
            read_address_1 = 0;
            read_address_2 = 0;
        #100;
            rst = 0;
            RegWrite = 1;
            write_address = 5;
            write_data = 23;
            read_address_1 = 0;
            read_address_2 = 0;
		#100;
            rst = 0;
            RegWrite = 0;
            write_address = 0;
            write_data = 0;
            read_address_1 = 4;
            read_address_2 = 5;
         #100;
            rst = 1;
            RegWrite = 0;
            write_address = 0;
            write_data = 0;
            read_address_1 = 0;
            read_address_2 = 0;
	    $finish;
    end

endmodule