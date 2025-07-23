module dynamics_tb_old();
	reg clk, rst;
	reg [15:0] sample;
	reg [7:0] multiplier;
	wire [15:0] sample_out;

	dynamics DUT (.clk(clk), .rst(rst), .sample_in(sample), .curr(5'd31), .start(5'd10), .multiple(multiplier), .sample_out(sample_out));

	initial begin
		clk = 1; #5 clk = 0;
		forever begin
			$display("rst=%b, ns=%b, sin%b, multiple=%b sout=%b", rst, DUT.new_sample, sample, multiplier, sample_out);
			#5 clk = 1; #5 clk = 0;
		end
	end

	initial begin
		#10 rst = 0;
		#10 rst = 1; sample = 16'b0001010101010101; 
		#10 rst = 0; 

        #10 multiplier = 8'b00000000;
        #10 multiplier = 8'b00000001;
        #10 multiplier = 8'b00000010;
        #10 multiplier = 8'b00000011;

		#10 sample = 16'd0; multiplier = 8'b10000000;		
		#10 multiplier = 8'b01111101;
		#10 multiplier = 8'b01111010;
		#10 multiplier = 8'b01111000;

		#10 multiplier = 8'b00110011;
		#10 multiplier = 8'b00110010;
		#10 multiplier = 8'b00110001;
		#10 multiplier = 8'b00110000;
		
		#10
		$stop;
		 
	end

endmodule