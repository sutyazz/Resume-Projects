module mcu(
    input clk,
    input reset,
    input play_button,
    input next_button,
    output play,
    output reset_player,
    output [1:0] song,
    input song_done
);

    assign reset_player = song_done || next_button;
    dffre playSong(.clk(clk), .r(reset || reset_player), .en(play_button), .d(~play), .q(play));

    reg [1:0] next_song;
    always @(*) begin
        case (song)
            2'b00: next_song = 2'b01;
            2'b01: next_song = 2'b10;
            2'b10: next_song = 2'b11;
            2'b11: next_song = 2'b00;
            default: next_song = 2'b00;
        endcase
    end
    dffre #(2) nextSong(.clk(clk), .r(reset), .en(reset_player), .d(next_song), .q(song));

endmodule
