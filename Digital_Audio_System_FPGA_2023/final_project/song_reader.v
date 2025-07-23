module song_reader(input clk,
			input reset,
			input play,
			input beat,
			input rewind,
			input fastforward,
			input wire [1:0] song, 
			input note_done,
			output reg song_done,
			output reg [17:0] note,
			output reg [17:0] duration,
			output advance,
			output new_note);
			
    wire [6:0] curr_note;
    wire [6:0] next_note;
    //get song address
    wire [8:0] addr = {song, curr_note - 1}; 
    wire [2:0] state;
    wire [15:0] songRom;
    wire restOrPlay;
    wire[5:0] current;
    wire[5:0] noteDurration;
    
    reg  [2:0] next;
    reg inc;
    reg [5:0] n1, d1, n2, d2, n3, d3, adv; //handle reading up to three notes to form a chord
			
			
	dffr #(7) notes( .clk(clk), .r(reset), .d(next_note), .q(curr_note));	
	
	//if increment check if song is done(set to 0s). if song is not done check if we are rewinding( mvoe note back 1) or playing forward(add 1 to note)
	assign next_note = inc ? (song_done ? 6'd0 : (rewind ? curr_note - 1'b1 : curr_note + 1'b1)) : curr_note;
	
	dffr #(3) states(.clk(clk), .r(reset), .d(next), .q(state));
	//get the song to be read
	song_rom songR(.clk(clk), .addr(addr), .dout(songRom) );
	//check if we are resting or still capturing notes
	assign restOrPlay = songRom[15];
	//capture the note and its respective durration 
	assign {current, noteDurration} = songRom[14:3];
	
	

`define WAIT 3'b000
`define NOTE1 3'b001
`define NOTE2 3'b010
`define NOTE3 3'b011
`define ADV 3'b100

always @(*) begin
	case(state)
		`WAIT: begin
            next = play ? (restOrPlay ? `ADV : `NOTE1) : `WAIT;
            inc = play ? 1'b1 : 1'b0;
            note = 18'd0;
            duration = 18'd0;
    end

		`NOTE1: begin    
            next = play ? `NOTE2 : `WAIT;
            n1 = current;
            d1 = (rewind || fastforward) ? (noteDurration>>2) : noteDurration;
            inc = 1'b1;
    end

    `NOTE2: begin
            next = play ? `NOTE3 : `WAIT;
            n2 = current;
            d2 = (rewind || fastforward) ? (noteDurration>>2) : noteDurration;
            inc = 1'b1;
			end

		`NOTE3:  begin	
            next = play ? `ADV : `WAIT;
            n3 = current;
            d3 = (rewind || fastforward) ? (noteDurration>>2) : noteDurration;
            inc = 1'b1;
			end

		`ADV: begin
				next = play ? (advance ? `NOTE1 : `ADV) : `WAIT ;
                adv = (rewind || fastforward) ? (noteDurration>>2) : noteDurration;
                note = {n1, n2, n3};
                duration = {d1, d2, d3};
                inc = advance ? 1'b1 : 1'b0;
             
			end
		default: begin
		        next = `WAIT;
			end
	endcase
end

wire [5:0] advance_t, next_advance_t;
    dffre #(.WIDTH(6)) dff_advance_t (
        .clk(clk),
        .r(reset),
        .en(play && (state == `ADV)),
        .d(next_advance_t),
        .q(advance_t)
    );
    assign next_advance_t = (reset || advance)
                        ? adv : (beat ? advance_t - 1 : advance_t);

    assign advance = (advance_t == 6'b0);
    
    assign new_note  = (advance_t == adv && (state == `ADV)) ? 1'b1 : 1'b0;
    
    //check if song is done regardless of direction it is being played in
    always @(*) begin
        case(song)
            2'b00: song_done = rewind ? ((addr == 9'd0) ? 1'b1 : 1'b0) : ((addr == 9'd127) ? 1'b1 : 1'b0);
            2'b01: song_done = rewind ? ((addr == 9'd128) ? 1'b1 : 1'b0) : ((addr == 9'd255) ? 1'b1 : 1'b0);
            2'b10: song_done = rewind ? ((addr == 9'd256) ? 1'b1 : 1'b0) : ((addr == 9'd383) ? 1'b1 : 1'b0);
            2'b11: song_done = rewind ? ((addr == 9'd384) ? 1'b1 : 1'b0) : ((addr == 9'd511) ? 1'b1 : 1'b0);

        endcase
    end
       


endmodule
