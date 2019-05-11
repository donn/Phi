`timescale 1ns/1ns

module mips_32_bit_tb;

	//Inputs
	reg clk;
    reg rst;

	//Outputs


    //Instantiation of Unit Under Test
        mips_32_bit uut(
            .clk(clk),
            .rst(rst)
        );

	initial begin
	//Inputs initialization
		clk = 0;
        rst = 1;
	end

    always 
    #5  clk = ~clk; 

    initial begin
	//Wait for the reset
		#100;
	    $finish;
    end
	

endmodule