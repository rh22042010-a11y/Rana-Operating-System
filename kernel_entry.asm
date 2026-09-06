[bits 32]
global _start       ; লিংকারকে বলে দেওয়া হলো এখান থেকেই শুরু

[extern main]       ; C ফাইলের main ফাংশনটিকে খুঁজছি

_start:
    call main       ; C কার্নেলের main ফাংশনটি কল করো
    jmp $           ; কাজ শেষ হলে এখানেই আটকে থাকো
