module dynamics_old (
	input clk,
	input rst,
	input [15:0] sample_in,
	input [5:0]	curr,
	input [5:0]	start,
	input [7:0]	multiple,
	output [15:0] sample_out
);

	wire [5:0] counter = start - curr;

    reg [15:0] temp1;
    reg [31:0] decayed_sample_reg, temp2;

     always @(*) begin
          temp1 = ~sample_in + 16'b1;
          temp2 = {temp1, 7'b0} * multiple;
          if(sample_in[15]) begin
               decayed_sample_reg = ~temp2 + 32'b1;
          end
          else begin
               decayed_sample_reg = {sample_in, 7'b0} * multiple;
          end
     end
     
	assign sample_out = (rst || counter == 7'd0) ? sample_in : decayed_sample_reg [31:16];

endmodule