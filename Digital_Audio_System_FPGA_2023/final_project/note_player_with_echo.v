module note_player(
    input clk,
    input reset,
    input play_enable,  // When high we play, when low we don't.
    input [5:0] note_to_load,  // The note to play
    input [5:0] duration_to_load,  // The duration of the note to play
    input load_new_note,  // Tells us when we have a new note to load
    output done_with_note,  // When we are done with the note this stays high.
    input beat,  // This is our 1/48th second beat
    input generate_next_sample,  // Tells us when the codec wants a new sample
    output [15:0] sample_out,  // Our sample output
    input new_frame, // necessary for codec in dynamics
    output new_sample_ready  // Tells the codec when we've got a sample
);

    wire [19:0] step_size;
    wire [5:0] freq_rom_in;

    dffre #(.WIDTH(6)) freq_reg (
        .clk(clk),
        .r(reset),
        .en(load_new_note),
        .d(note_to_load),
        .q(freq_rom_in)
    );

    frequency_rom freq_rom(
        .clk(clk),
        .addr(freq_rom_in),
        .dout(step_size)
    );

    wire [15:0] temp_sample_out;
    sine_reader sine_read(
        .clk(clk),
        .reset(reset),
        .step_size(step_size),
        .generate_next(play_enable && generate_next_sample),
        .sample_ready(new_sample_ready),
        .sample(temp_sample_out)
    );

    wire [5:0] state, next_state;
    dffre #(.WIDTH(6)) state_reg (
        .clk(clk),
        .r(reset),
        .en((beat || load_new_note) && play_enable),
        .d(next_state),
        .q(state)
    );

    assign next_state = (reset || done_with_note || load_new_note)
                        ? duration_to_load : state - 1;
    assign done_with_note = (state == 6'b0) && beat;
	
    // Echo module
    reg [15:0] delayed_sample, delay_array [9600:0];  // 200ms delay
    reg index = 1'b0;  // index i stores values of 9600+ith released echo note
    always @(*) begin // repeats every time inputs change
            delay_array[index] = temp_sample_out;
	    index = reset ? 9600 : ((index == 0) ? 9600 : index - 1); // counts down index every time
	    delayed_sample = delay_array[index]; 
    end

    assign sample_out = temp_sample_out + (delayed_sample >> 1); // attenuates to 1/2 volume by shifting right 1
	
    // Dynamics Instantiation
	// dynamics dyn(
	// 	.clk(clk),
	// 	.reset(reset),
	// 	.sample_start(temp_sample_out),
	// 	.done_with_note(done_with_note), 
	// 	.new_sample_ready(new_sample_ready),
	// 	.note_duration(duration_to_load),
	// 	.sample_out(sample_out),
	// 	.new_frame(new_frame)				
	// );

endmodule
