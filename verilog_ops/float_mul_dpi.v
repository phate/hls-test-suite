module float_mul_dpi (
  input  [31:0] a,
  input  [31:0] b,
  output [31:0] out
);
  import "DPI-C" function void float_mul_dpi_c(input bit [31:0] a, input bit [31:0] b, output bit [31:0] out);

  // Call the DPI-C function
  always_comb begin
    float_mul_dpi_c(a, b, out);
  end
endmodule
