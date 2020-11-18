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