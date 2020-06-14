module debug_main #(
   parameter DATA_WIDTH = 512,
   parameter ADDR_WIDTH = 64,
   parameter MONITOR_WIDTH = 64
)(
  
   input clk,
   input rst,
   input start,
   
   // Configuração
   input [ADDR_WIDTH-1:0] input_address_in,
   input [ADDR_WIDTH-1:0] output_address_in,
   input [ADDR_WIDTH-1:0] status_addres_in,
   input [ADDR_WIDTH-1:0] samples_size,
   input [ADDR_WIDTH-1:0] read_delay_window,
   
   // Leitura
   input rd_available,
   input rd_valid,
   input [DATA_WIDTH-1:0] rd_data,  
   
   output reg [ADDR_WIDTH-1:0] rd_addr,
   output reg req_rd,
   
   // Escrita
   input wr_available,
   input wr_valid,
   
   output reg [ADDR_WIDTH-1:0] wr_addr,
   output reg req_wr,
   output reg [DATA_WIDTH-1:0] wr_data   
);


   reg [2:0] fsm_req_rd;
   reg [2:0] fsm_receive;
   reg [2:0] fsm_wr;
   reg [31:0] internal_sleep;
   reg fsm_rd_fifo;
   
   reg [ADDR_WIDTH-1:0] clock_counter;
   reg [ADDR_WIDTH-1:0] saves_counter;
   reg [MONITOR_WIDTH-1:0] request_counter;
   
   reg flag_new;
   reg [MONITOR_WIDTH-1:0] monitorada;
   reg [MONITOR_WIDTH-1:0] last_monitorada;
   reg wr_fifo;
   
   wire fifo_data_valid;
   wire [2*MONITOR_WIDTH+64-1:0] fifo_data_out;
   wire fifo_empty;
   
   
   reg [MONITOR_WIDTH*2-1+64:0]fifo_data_in;
   
   reg fifo_rd_en;
   reg flag_first;
   
   
   reg flag_waiter;
   
   always @(posedge clk) begin
       if (rst) begin
          fsm_req_rd <= 0;
          req_rd <= 0;
          request_counter <= 0;
          internal_sleep <=0;
          flag_waiter<=0;
       end    
       else begin
          req_rd <= 0;
          case(fsm_req_rd)
            0: begin
               if(start)begin
                  rd_addr <= input_address_in;
                  fsm_req_rd <= 1;
               end 
            end 
            1:begin
                if (rd_available) begin
                    flag_waiter<=1;
                end
                if (flag_waiter) begin
                    if (internal_sleep<read_delay_window) begin
                        internal_sleep <= internal_sleep + 1;
                    end
                    else begin
                        internal_sleep<=0;
                        flag_waiter<=0;
                        fsm_req_rd<=2;
                    end 
                end 
            end
            
            2: begin
               if(saves_counter < samples_size)begin
                if(rd_available)begin
                    req_rd <= 1;
                    request_counter <= request_counter + 1;
                    fsm_req_rd<=1;
                end 
              end
              else begin
                fsm_req_rd <= 3;
              end                
            end 
            
            
            3: begin
             // fim dos pedidos de leitura
            end 
          endcase
          
       end 
   end
   
   
   
   
   
   
   always @(posedge clk) begin
     if(rst)begin
        fsm_receive <= 0;
        flag_new <= 0;
        monitorada<=0;
     end else begin
         flag_new <= 0;
         case(fsm_receive)
            0: begin
              if(start)begin
                 fsm_receive <= 1;
              end 
            end 
            1:begin
              if(saves_counter < samples_size)begin
                  if(rd_valid)begin
                     monitorada <= rd_data[31:0];
                     flag_new <= 1;
                  end 
              end 
              else begin
                 fsm_receive <= 2;
              end 
            end 
            2: begin
              // Fim do recebimento
            end 
         endcase
     end 
   
   end 
   
   always @(posedge clk) begin
     if(rst)begin
       req_wr <= 0;
       fsm_wr <= 0;
     end 
     else begin
       req_wr <= 0;
       case(fsm_wr)
         0:begin
           if(start)begin
             wr_addr <=  output_address_in;
             fsm_wr <= 1;
           end 
         end 
         1:begin
           if(fifo_data_valid)begin
             wr_addr <= wr_addr + 1;
             req_wr <= 1;
             wr_data <= {320'd0,fifo_data_out};
           end 
           else  if(fsm_receive == 2)begin
              fsm_wr <= 2;
           end
         end 
         2:begin
            if(wr_available)begin
               wr_addr <= status_addres_in;
               req_wr <= 1;
               wr_data <= 512'd1;
               fsm_wr <= 3;
            end 
         end
         3:begin
          // FIM
         end 
       endcase
     end 
   end  
   
   always @(posedge clk)begin
     if(rst)begin
        last_monitorada <= 0;
        wr_fifo <= 0;
        flag_first<=0;
     end else begin
        wr_fifo<=0;
        if(flag_new)begin
           if (flag_first) begin
                wr_fifo <= |(last_monitorada ^ monitorada);
           end
           flag_first<=1'b1;
           last_monitorada <= monitorada;
           fifo_data_in <= {clock_counter,last_monitorada,monitorada};
        end 
     end 
     
   end 
   
   always @(posedge clk) begin
       if (rst) begin
           clock_counter <= 0;
           saves_counter<=0;

       end
       else begin
           if (start) begin
               clock_counter <= clock_counter + 64'd1;
               if (wr_fifo==1) begin
                    saves_counter<= saves_counter + 1;
               end 

           end
       end
   end 

     
   always @(posedge clk) begin
       if (rst) begin
        fifo_rd_en <= 0;
        fsm_rd_fifo<=0;
       end
       else begin
           fifo_rd_en <= 0;
           case(fsm_rd_fifo)
             0:begin
                if (~fifo_empty & wr_available) begin
                  fifo_rd_en <= 1;
                  fsm_rd_fifo <= 1;
                end 
             end 
             1:begin
               fsm_rd_fifo <= 0;
             end
           
           endcase
       end
   end 
 
  fifo
  #(
    .FIFO_WIDTH(2*MONITOR_WIDTH+64),
    .FIFO_DEPTH_BITS(10),
    .FIFO_ALMOSTFULL_THRESHOLD(4),
    .FIFO_ALMOSTEMPTY_THRESHOLD(4)
  )
  in_fifo
  (
    .clk(clk),
    .rst(rst),
    .we(wr_fifo),
    .din(fifo_data_in),
    .re(fifo_rd_en),
    .valid(fifo_data_valid),
    .dout(fifo_data_out),
    .count(),
    .empty(fifo_empty),
    .full(),
    .almostfull(),
    .almostempty()
  );
  

endmodule
