`phi "Test2.phi"

module testbench;

    reg[3:0] a, b;
    wire[3:0] s;
    wire cout;

    Adder adder(.a(a), .b(b), .s(s), .cout(cout));
    integer i;
    initial begin
        $dumpvars(0, testbench);
        #100;
        for (i = 0; i < 10; i = i + 1) begin
            a = $urandom;
            b = $urandom;
            #100;
        end
        $finish;
    end

endmodule