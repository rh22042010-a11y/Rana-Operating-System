[bits 32]           ; আমরা 32-bit প্রটেক্টেড মোডে কাজ করব
[extern main]       ; C ফাইলের main ফাংশনটিকে খুঁজছি

call main           ; C কার্নেলের main ফাংশনটি কল করো
jmp $               ; কাজ শেষ হলে এখানেই আটকে থাকো (Infinite loop)
