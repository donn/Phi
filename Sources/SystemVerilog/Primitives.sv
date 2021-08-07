module \Phi.Common.Register (
    clock,
    reset,
    resetValue,
    enable,
    data,
    out
);
    parameter from = 0;
    parameter to = 0;

    input clock, reset, enable;
    input[from:to] resetValue;
    input[from:to] data;
    output[from:to] out;

    reg[from:to] internal;

    always @ (posedge clock or posedge reset) begin
        if (reset) begin
            internal <= resetValue;
        end else begin
            if (enable) begin
                internal <= data;
            end
        end
    end

    assign out = internal;

endmodule

module \Phi.Common.Latch (
    condition,
    enable,
    data,
    out
);
    parameter from = 0;
    parameter to = 0;

    input condition, enable;
    input[from:to] data;
    output[from:to] out;

    reg[from:to] internal;

    always @ (condition or data) begin
        if (condition && enable) begin
            internal <= data;
        end
    end

    assign out = internal;

endmodule

module \Phi.Common.Eq (
    a,
    b,
    y
);
    parameter width = 1;

    input[width-1:0] a;
    input[width-1:0] b;

    output y;

    assign y = (a == b);
endmodule

module \Phi.Common.Decoder (
    in,
    out
);
    parameter width = 1;
    localparam n = 1 << width;

    input[width-1:0] in;
    output[n-1:0] out;

    genvar i;
    for (i = 0; i < n; i = i + 1) begin: genblock
        wire[width-1:0] current = i;
        \Phi.Common.Eq #(.width(width)) eq( .a(current), .b(in), .y(out[i]) );
    end
endmodule