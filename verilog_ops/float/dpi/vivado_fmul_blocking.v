// Generated by CIRCT firtool-1.109.0
module vivado_fmul_blocking(	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:458:7
  input         s_axis_a_tvalid,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:460:29
  output        s_axis_a_tready,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:461:29
  input  [31:0] s_axis_a_tdata,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:462:28
  input         s_axis_b_tvalid,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:463:29
  output        s_axis_b_tready,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:464:29
  input  [31:0] s_axis_b_tdata,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:465:28
  output        m_axis_result_tvalid,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:466:34
  input         m_axis_result_tready,	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:467:34
  output [31:0] m_axis_result_tdata	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:468:33
);

  wire _m_axis_result_tvalid_output = s_axis_a_tvalid & s_axis_b_tvalid;	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:470:46
  wire _s_axis_b_tready_T = m_axis_result_tready & _m_axis_result_tvalid_output;	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:470:46, :471:45
  float_mul_dpi mul (	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:473:21
    .a   (s_axis_a_tdata),
    .b   (s_axis_b_tdata),
    .out (m_axis_result_tdata)
  );	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:473:21
  assign s_axis_a_tready = _s_axis_b_tready_T;	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:458:7, :471:45
  assign s_axis_b_tready = _s_axis_b_tready_T;	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:458:7, :471:45
  assign m_axis_result_tvalid = _m_axis_result_tvalid_output;	// git/chisel-template/src/main/scala/hls_float/hls_float.scala:458:7, :470:46
endmodule


