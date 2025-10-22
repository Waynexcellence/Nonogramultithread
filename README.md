# Nonogramultithread
trying to solve a nonogram puzzle in C with multithread.

# Requirement
- gcc (Rev2, Built by MSYS2 project) 14.2.0
# 輸入/輸出
- 第一個整數是高度，第二個整數是寬度
- 接著輸入`row`的提示，方向從上到下
- 最後輸入`col`的提示，方向從左到右
- 可以參考 `test.in`、`test.out`
# 使用方法
- 使用`call build`，可以編譯出`main.exe`
- 使用`.\main < test.in > test.out`可以計算答案，產生的`log`是每個thread的計算過程
- 也可使用`.\main`慢慢輸入，程式會根據之前的輸入顯示目前的題目樣貌