`timescale 1ns/1ns

module processor_control_tb;

	//Inputs
	reg[31:0] pc;

	//Outputs
	wire[31:0] instruction;

    //Instantiation of Unit Under Test
        instruction_memory uut(
            .pc(pc),
            .instruction(instruction)
        );

	initial begin
	//Inputs initialization
		pc = 0;
	end


    initial begin
	//Wait for the reset
		#100;
			pc = 4;
		#100;
			pc = 8;
		#100;
			pc = 12;
	    $finish;
    end
	

endmodule