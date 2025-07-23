module sine_reader(
    input clk,
    input reset,
    input [19:0] step_size,
    input generate_next,

    output sample_ready,
    output wire [15:0] sample
);

    wire reading;
    wire [21:0] currentSum;
    wire [21:0] nextSum;
    wire [15:0] dout;
    
    dffr #(1) state_reg1(.clk(clk), .r(reset), .d(generate_next), .q(reading));
    dffr #(1) state_reg2(.clk(clk), .r(reset), .d(reading & !reset), .q(sample_ready));
    dffr #(22) state_reg3(.clk(clk), .r(reset), .d(nextSum), .q(currentSum));
    
    assign nextSum = generate_next ? currentSum + step_size : currentSum;

    sine_rom srom(.clk(clk), .addr(nextSum[20] ? 10'd1023-nextSum[19:10] : nextSum[19:10]), .dout(dout));
    assign sample = currentSum[21] ? 16'd0-dout : dout;

endmodule
