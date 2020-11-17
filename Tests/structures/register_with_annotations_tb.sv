`phi "Test10.phi"

module testbench;
    reg clk;
    reg reset;

    always #1 clk = !clk;

    Counter counter(.clock(clk), .reset(reset));

    initial begin
        $dumpvars(0, testbench);
        clk = 0;
        reset = 1;
        #100;
        reset = 0;
        #1000;
        $finish;
    end

endmodule