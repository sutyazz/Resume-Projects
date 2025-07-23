module wave_display (
    input clk,
    input reset,
    input [10:0] x,  // [0..1279]
    input [9:0]  y,  // [0..1023]
    input valid,
    input [7:0] read_value,
    input read_index,
    output reg [8:0] read_address,
    output wire valid_pixel,
    output reg [7:0] r, g, b
);

reg [7:0] read_value_adj;
always @(*) begin
    read_value_adj = (read_value >> 1) + 8'd32;
end

assign valid_pixel = valid && (x[9]^x[8]) && ~y[9];

always @(*) begin
    casex(x)
        11'b000xxxxxxxx : read_address = 9'd0;
        11'b001xxxxxxxx : read_address = {read_index, 1'b0, x[7:1]};
        11'b010xxxxxxxx : read_address = {read_index, 1'b1, x[7:1]};
        11'b011xxxxxxxx : read_address = 9'd0;
        default: read_address = 9'd0;
    endcase
end

wire [8:0] addr_prev;
dffr #(9) addr_dff (.clk(clk), .r(reset), .d(read_address), .q(addr_prev)); // extra clk cycle

wire update;
assign update = (read_address != addr_prev);

wire [7:0] value_prev;
dffre #(8) send_to_ram (.clk(clk), .r(reset), .en(update), .d(read_value_adj), .q(value_prev));

    reg [7:0] dist, abs_dist;
always @(*) begin
    if(valid_pixel && (((y[8:1]<=read_value_adj) && (y[8:1]>=value_prev)) || ((y[8:1]<=value_prev) && (y[8:1]>=read_value_adj)))) begin
        // calculate distance from line connecting adjacent samples
        dist = (y[8:1] - value_prev)*(x[7:0] - addr_prev[7:0]) -
                   (x[7:0] - addr_prev[7:0])*(read_value_adj - value_prev);
        abs_dist = (dist < 0) ? -dist : dist;

        // calculate color values based on distance from line
        if (abs_dist <= 8'h10) begin
            r = 8'hFF;
            g = 8'hFF; // green fades from 0x00 to 0xFF
            b = 8'hFF;
        end
    end else begin
        r = 8'h00;
        g = 8'h00;
        b = 8'h00;
    end
end

endmodule
