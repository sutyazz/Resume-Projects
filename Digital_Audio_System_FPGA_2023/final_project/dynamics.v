module dynamics(
    input clk,
    input reset, 
	input [5:0] note_duration,		
	input [15:0] sample_start,					
	input new_sample_ready,
	input done_with_note,			
	input new_frame,
    output [15:0] sample_out
);

	wire generate_next_sample;
	wire [15:0] sample;
	
	codec_conditioner codec_conditioner(
        .clk(clk),
        .reset(reset),
        .new_sample_in(sample_start),
        .latch_new_sample_in(new_sample_ready),
        .generate_next_sample(generate_next_sample),
        .new_frame(new_frame),
        .valid_sample(sample)
    );

    // Beat that should pulse 256 times per second?
	wire beat;
    wire [2:0] beat_count;
    dffre #(3) counterer (
        .clk(clk),
        .en(generate_next_sample),
        .r(reset | (beat_count == 3'd4)),
        .d(beat_count + 1'b1),
        .q(beat_count)
    );
    assign beat = (beat_count == 3'd4);


	// Attack and Sustain Phase: 1/8th of total duration each (decay remaining 3/4th)
	
	reg  attack_done;
	reg [15:0] attack_final_temp;
	wire [15:0] attack_final;
    wire [13:0] sustain;
    wire decay;
    
	wire [13:0] attack_duration, attack_subtract;
	
    wire[13:0] temp_duration;
    assign temp_duration = {8'd0, note_duration};
	
	// Each attach is 1/64th of the total duration, and adds up linearly 8 times.
	assign attack_duration = temp_duration << 2; 

	// We want the hold to be 8 times as large at the attack_duration so that it also
	// takes a full 1/8th of the duration.
	assign decay = (sustain == attack_duration << 3);
	
	// This keeps track of how many times we should have added 1/8th of the amplitude
	// of the total sample to our output sample.
    wire [4:0] attack_count;
	dffre #(.WIDTH(5)) attack(
		.clk(clk),
		.r(reset || done_with_note),
		.d(attack_count + 1),
		.q(attack_count),
		.en(beat && (attack_duration - attack_subtract == 1) && !attack_done)
	);
	
	// When this reaches a certain limit i.e. 1/64th of the total note_duration,
	// it will enable the count flop (as part of the enable).
	dffre #(.WIDTH(14)) attack_sub(
		.clk(clk),
		.r(reset || done_with_note || attack_duration - attack_subtract == 0),
		.en(beat),
		.d(attack_subtract + 1),
		.q(attack_subtract)
	);
	
	// This takes care of the Sustain phase by waiting until the attack phase is
	// done, then it counts up to 1/8th of the note duration.
	dffre #(.WIDTH(14)) sustain_phase(
		.clk(clk),
		.r(reset || done_with_note),
		.en(beat && attack_done && !decay),
		.d(sustain + 1),
		.q(sustain)
	);

    wire[15:0] sample_subtract_8, sample_subtract_4, sample_subtract_2;

    assign sample_subtract_2 = $signed(sample) >>> 1'd1;
	assign sample_subtract_4 = $signed(sample) >>> 2'd2;
	assign sample_subtract_8 = $signed(sample) >>> 2'd3;

	always @(*) begin
		case (attack_count)
		1:			{attack_final_temp, attack_done} = {sample_subtract_8, 1'b0};
		2:			{attack_final_temp, attack_done} = {sample_subtract_4, 1'b0};
		3:			{attack_final_temp, attack_done} = {sample_subtract_4 + sample_subtract_8, 1'b0};
		4:			{attack_final_temp, attack_done} = {sample_subtract_2, 1'b0};
		5:			{attack_final_temp, attack_done} = {sample_subtract_2 + sample_subtract_8, 1'b0};
		6:			{attack_final_temp, attack_done} = {sample_subtract_2 + sample_subtract_4, 1'b0};
		7:			{attack_final_temp, attack_done} = {sample - sample_subtract_8, 1'b0};
		8:			{attack_final_temp, attack_done} = {sample, 1'b1};
		default: {attack_final_temp, attack_done} = {16'd0, 1'b0};
		endcase
	end
	
	assign attack_final = attack_final_temp;
	
	// Decay Phase

	wire [13:0] new_duration;
	wire [13:0] decay_duration;
	wire [13:0] subtractor; // How much we should be subtracting from sample for decay
	wire [13:0] flop_duration;
	
    wire zero;
    wire [13:0] result;
	assign zero = result == 0 ;

	// Make decay last for only 3/4 of the duration.
	assign decay_duration = temp_duration + temp_duration + temp_duration;
	assign flop_duration = decay_duration >> 1;

	// Keeps track of how much we should be subtracting from sample.
    wire [4:0] sub_count;
	dffre #(.WIDTH(5)) counter(
		.clk(clk),
		.r(reset || done_with_note),
		.d(sub_count + 1),
		.q(sub_count),
		.en(zero && subtractor != 0 && decay)
	);
	
	// Make the duration longer each time count is incremented to simulate an exponential decay.
	dffre #(.WIDTH(14)) exp_decay(
		.clk(clk),
		.r(reset || done_with_note),
		.d(flop_duration << sub_count),
		.q(new_duration),
		.en(beat && decay)
	);
	
	// This is a counter that when it reaches a certain limit (new_duration)
	// enables the counter flop, which in turn increments count and it starts over.
	dffre #(.WIDTH(14)) subtract_by(
		.clk(clk),
		.r(reset || done_with_note || zero),
		.d(subtractor + 1),
		.q(subtractor),
		.en(beat && decay)	
	);

	// Whether it's time to drop the volume of the note again (or not)
    
	assign result = new_duration - subtractor;
    
	reg [15:0] final_temp;
	always @(*) begin
		case (sub_count)
            1:	final_temp = sample - sample_subtract_8;
            2:	final_temp = sample - sample_subtract_4;
            3:	final_temp = sample - sample_subtract_4 - sample_subtract_8;
            4:	final_temp = sample - sample_subtract_2;
            5:	final_temp = sample - sample_subtract_2 - sample_subtract_8;
            6:	final_temp = sample - sample_subtract_2 - sample_subtract_4;
            7:	final_temp = sample_subtract_8;
            8:	final_temp = 1'b0;
		    default: final_temp = sample;
		endcase
	end

	assign sample_out = attack_done ? final_temp : attack_final;

endmodule
