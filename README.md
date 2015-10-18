# big-mode-key

Windows only.

Превращает клавишу Space в Mod клавишу. Если нажать Space просто так, то отрабатывает как просто Пробел. Если зажать и нажать какую-то еще клавишу то отработает сочитаение клавишь прописанное а файле key_binds. Теперь раскладку стрелок из vim(HJKL) можно использовать во всей Windows.

В файле key_binds можно использовать как символьные имена клавишь(config.c), так и виртуальный код клавиши.

Making Space key as Mod key. Single press Space its simple Space. Holding Space and pressing other keys making key combinations from file "key_binds". Now you can use vim arrows(HJKL) in all Windows.

In "key_binds" you can use symbolic names for Keys ( see config.c ), or decimal virtual code.
(Sorry for my bad English)

`build.bat` must be run under Visual Studio Command Promt.