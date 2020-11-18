`phi "Tests/structures/register_with_annotations.phi"

module testbench;
    reg clk;
    reg reset;
    reg increment;

    always #1 clk = !clk;

    Counter counter(.clock(clk), .reset(reset), .enable(1'b1), .increment(increment));

    initial begin
        $dumpvars(0, testbench);
        clk = 0;
        increment = 1;
        reset = 1;
        #100;
        reset = 0;
        #1000;
        increment = 0;
        #1000;
        
        $finish;
    end

endmodule