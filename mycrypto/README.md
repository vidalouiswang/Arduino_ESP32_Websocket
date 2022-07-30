At first this component only have SHA1 and Base64 for websocket use
(because I don't know how to use offical part, and don't know it will use hardware accleration or not, so I made this). 
Then I add SHA256 and AES-256-CBC to it, so it could be use at other places.

最初这个组件只有SHA1和Base64供websocket使用(因为我不会用官方库，也不知道是软件实现还是使用了硬件加速器
(因为一开始是使用1.0.6，底层是esp-idf 3，后来的版本迁移esp-idf 4导致官方组件很乱，我也懒得找了，所以自己写了一个)，所以自己调用硬件加速才有了这个组件)。
后来我又添加了SHA256和AES-256-CBC，所以它也可以在其他地方使用。
