module dynamics_tb();
	reg [5:0] note_duration;
	reg clk;
	reg reset;
	reg [15:0] sample;
	reg done_with_note;
	reg new_sample_ready;
	reg generate_next_sample;
	wire [15:0] final_sample;
	
	initial begin
        clk = 1'b0;
        reset = 1'b1;
        repeat (4) #5 clk = ~clk;
        reset = 1'b0;
        forever #5 clk = ~clk;
    end
	
	dynamics_new dut(
		.note_duration(note_duration),
		.clk(clk),
		.reset(reset),
		.sample_start(sample),
		.done_with_note(done_with_note),
		.new_sample_ready(new_sample_ready),
		.final_sample(final_sample)
	);
	
	initial begin
        reset = 1;
        note_duration = 6'd3;
        sample = 16'd0;
        new_sample_ready = 1'b0;
        generate_next_sample = 1'b1;
        #30
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);

        done_with_note = 1'b1;
        #30
        
        // testing main sample
        reset = 0;					
        note_duration = 6'd3;
        sample = 16'd1000; 
        new_sample_ready = 1'b0;
        #100
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);
        
        #2000
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);

        #5000
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);
        
        done_with_note = 1'b0;
        #30

        done_with_note = 1'b1;
        #30

        // Testing with a negative sample to make sure the program works with signed #s.
        note_duration = 6'd24;
        sample = 16'd0 - 16'd1000;
        new_sample_ready = 1'b1;
        generate_next_sample = 1'b0;
        #10
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);
        
        #2000
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);

        #5000
        $display("Duration: %b, new_sample_ready : %b, final_sample: %b",
        note_duration, new_sample_ready, final_sample);
        
        $stop;
	end
	
endmodule
