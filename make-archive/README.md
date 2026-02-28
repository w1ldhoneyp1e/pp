tar -xvf block-tar

gunzip -k block.txt_0.gz

./make-archive -S block-new ./tests/pptx/reg_1.pptx ./tests/pptx/reg_2.pptx ./tests/pptx/reg_3.pptx ./tests/pptx/reg_4.pptx ./tests/pptx/reg_5.pptx ./tests/pptx/reg_6.pptx ./tests/pptx/reg_7.pptx ./tests/pptx/reg_8.pptx ./tests/pptx/reg_9.pptx ./tests/pptx/reg_10.pptx ./tests/pptx/reg_11.pptx ./tests/pptx/reg_12.pptx ./tests/pptx/reg_13.pptx ./tests/pptx/reg_14.pptx ./tests/pptx/reg_15.pptx

./make-archive -P 4 block-new ./tests/pptx/reg_1.pptx ./tests/pptx/reg_2.pptx ./tests/pptx/reg_3.pptx ./tests/pptx/reg_4.pptx ./tests/pptx/reg_5.pptx ./tests/pptx/reg_6.pptx ./tests/pptx/reg_7.pptx ./tests/pptx/reg_8.pptx ./tests/pptx/reg_9.pptx ./tests/pptx/reg_10.pptx ./tests/pptx/reg_11.pptx ./tests/pptx/reg_12.pptx ./tests/pptx/reg_13.pptx ./tests/pptx/reg_14.pptx ./tests/pptx/reg_15.pptx