// file: SuccessiveApproximationControl_tb.v
// author: @skyus
// Testbench for SuccessiveApproximationControl

`timescale 1ns/1ns

`ifdef _phi
`phi "SAR.phi"
`else
`include "SAR.v"
`endif

module SuccessiveApproximationControl_tb;

	//Inputs
	reg clk;
	reg go;
	reg cmp;
	reg reset;


	//Outputs
	wire [15: 0] value;
	wire [15: 0] result;
	wire valid;
	wire sample;
	
	//Testing
    real try = 11037.42;
    wire error = valid && (try != result);

    always begin
        #1;
        clk <= ~clk;
        cmp <= try < value;
    end


	//Instantiation of Unit Under Test
	SuccessiveApproximationControl uut (
		.clk(clk),
		.go(go),
		.cmp(cmp),
		.reset(reset),
		.value(value),
		.result(result),
		.valid(valid),
		.sample(sample)
	);

	initial begin
		$dumpvars(0, SuccessiveApproximationControl_tb);
	//Inputs initialization
		clk = 0;
		go = 0;
		cmp = 0;
		reset = 1;


	//Wait for the reset
		#100;
		reset = 0;
		
		go = 1;
		#1000;
		$finish();
	end

endmodule