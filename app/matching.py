from io import BytesIO
from aiogram.types import Message, BufferedInputFile
from app.keyboards import partner_navigation_keyboard


async def show_partner_profile(message: Message, users, index: int):
    partner = users[index]
    user_profile_photo = await message.bot.get_user_profile_photos(partner["tg_id"], limit=1)

    file_bytes = None
    if user_profile_photo.total_count > 0:
        photo = user_profile_photo.photos[0][-1]
        file = await message.bot.download(photo.file_id)
        file_bytes = BytesIO(file.read())
        file_bytes.seek(0)

    match_percent = partner.get("match_percent", 0)

    match_type = (
        "🎯 100% совпадение" if match_percent == 100
        else f"🤝 Совпадение: {match_percent}%"
    )

    profile_text = (
        f"{match_type}\n\n"
        f"Имя: {partner['in_bot_name']} \n"
        f"Возраст: {partner['years']} \n"
        f"Контакт: @{partner['tg_username']}"
    )
    keyboard = partner_navigation_keyboard(index, len(users))

    if file_bytes:
        await message.bot.send_photo(
            chat_id=message.chat.id,
            photo=BufferedInputFile(file_bytes.read(), filename="avatar.jpg"),
            caption=profile_text,
            reply_markup=keyboard
        )
    else:
        await message.answer(profile_text, reply_markup=keyboard)

