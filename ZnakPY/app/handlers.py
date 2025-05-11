import os
from io import BytesIO
from aiogram import Router, F
from aiogram.filters import CommandStart
from aiogram.fsm.context import FSMContext
from aiogram.types import Message, BufferedInputFile, CallbackQuery

import app.database.requests as rq
import app.keyboards as kb
from app.models import RegistrationState
from app.questions import questions, questions_wanted

router = Router()


# ---------- Команды ----------
@router.message(CommandStart())
async def cmd_start(message: Message):
    if message.from_user.id == int(os.getenv('ADMIN_ID')):
        await message.answer("Привет админ!", reply_markup=kb.admin_menu)
        return

    user = await rq.get_user(message.from_user.id)
    if not user:
        await message.answer(
            "Вы у нас первый раз! Нажмите 'Регистрация 🚀' ниже, чтобы отправить ваш контакт.",
            reply_markup=kb.reg_keyboard
        )
    else:
        await message.answer("Добро пожаловать!", reply_markup=kb.menu)


# ---------- Регистрация ----------
@router.message(F.text == 'Регистрация 🚀')
async def request_contact(message: Message):
    await message.answer("Пожалуйста, поделитесь контактом:", reply_markup=kb.reg_keyboard)


@router.message(F.contact)
async def handle_contact(message: Message):
    contact = message.contact
    await rq.add_user(
        tg_id=message.from_user.id,
        username=message.from_user.username or "",
        first_name=contact.first_name or None,
        last_name=contact.last_name or None,
        number=contact.phone_number
    )
    await message.answer(
        "Спасибо за регистрацию! Теперь давайте заполним вашу анкету.",
        reply_markup=kb.opros_keyboard
    )


# ---------- Анкета ----------
@router.message(F.text == 'Пройти опрос 🤙')
async def start_survey(message: Message, state: FSMContext):
    await message.answer("Как тебя зовут?", reply_markup=None)
    await state.set_state(RegistrationState.waiting_for_name)


@router.message(RegistrationState.waiting_for_name)
async def process_name(message: Message, state: FSMContext):
    await state.update_data(name=message.text)
    await message.answer("Выберите ваш пол", reply_markup=kb.sex)
    await state.set_state(RegistrationState.waiting_for_sex)


@router.message(RegistrationState.waiting_for_sex)
async def process_sex(message: Message, state: FSMContext):
    sex_map = {
        "Мужской 🙋‍♂️": "man",
        "Женский 🙋‍♀️": "woman"
    }

    if message.text not in sex_map:
        await message.answer("Пожалуйста, выберите пол из кнопок ниже")
        return

    await state.update_data(sex=sex_map[message.text])
    await message.answer("Сколько тебе лет?", reply_markup=None)
    await state.set_state(RegistrationState.waiting_for_age)


@router.message(RegistrationState.waiting_for_age)
async def process_age(message: Message, state: FSMContext):
    try:
        age = int(message.text)
        if not 16 <= age <= 100:
            raise ValueError
    except ValueError:
        await message.answer("Пожалуйста, введите корректный возраст (16-100)")
        return

    user_data = await state.get_data()
    await rq.update_profile(
        tg_id=message.from_user.id,
        in_bot_name=user_data.get('name'),
        sex=user_data.get('sex'),
        years=age
    )

    await message.answer(
        "Теперь давай заполним анкету о тебе и твоих предпочтениях!",
        reply_markup=kb.start_opros
    )
    await state.clear()


# ---------- Опрос ----------
@router.message(F.text.in_(['Пройти опросик 👻', 'Перепройти опрос 🔄']))
async def start_questions(message: Message, state: FSMContext):
    await state.update_data(your_answers=[], wanted_answers=[])
    await ask_question(message, state, 1)


async def ask_question(message: Message, state: FSMContext, question_id: int):
    if question_id in questions:
        await message.answer(
            questions[question_id]["question"],
            reply_markup=kb.get_question_keyboard(question_id)
        )
    else:
        await ask_wanted_question(message, state, 1,message.from_user.id)

async def ask_wanted_question(message: Message, state: FSMContext, question_id: int, user_id: int):
    if question_id in questions_wanted:
        await message.answer(
            questions_wanted[question_id]["question"],
            reply_markup=kb.get_wanted_question_keyboard(question_id)
        )
    else:
        data = await state.get_data()
        await rq.update_questionnaire(
            tg_id=user_id,
            unic_your_id=";".join(data.get("your_answers", [])),
            unic_wanted_id=";".join(data.get("wanted_answers", []))
        )

        if user_id == int(os.getenv('ADMIN_ID')):
            await message.answer("Готово админ", reply_markup=kb.admin_menu)
        else:
            await message.answer(
                "Видишь, не стоило бояться! Ты прошел регистрацию!",
                reply_markup=kb.menu
            )
        await state.clear()


@router.callback_query(F.data.startswith("answer_you_"))
async def handle_you_answer(callback: CallbackQuery, state: FSMContext):
    _, _, question_id, answer_index = callback.data.split("_")
    data = await state.get_data()
    answers = data.get("your_answers", [])
    answers.append(answer_index)
    await state.update_data(your_answers=answers)
    await callback.message.delete()
    await ask_question(callback.message, state, int(question_id) + 1)
    await callback.answer()


@router.callback_query(F.data.startswith("answer_wanted_"))
async def handle_wanted_answer(callback: CallbackQuery, state: FSMContext):
    _, _, question_id, answer_index = callback.data.split("_")
    data = await state.get_data()
    answers = data.get("wanted_answers", [])
    answers.append(answer_index)
    await state.update_data(wanted_answers=answers)
    await callback.message.delete()
    await ask_wanted_question(callback.message, state, int(question_id) + 1, callback.from_user.id)
    await callback.answer()


# ---------- Профиль ----------
@router.message(F.text == 'Моя анкета 🫵')
async def show_profile(message: Message):
    user = await rq.get_user(message.from_user.id)
    if not user:
        await message.answer(
            "Анкета не найдена. Сначала зарегистрируйтесь!",
            reply_markup=kb.reg_keyboard
        )
        return

    status = "✅ Участвует в поиске" if user['status'] else "❌ Не участвует"
    profile_text = (
        f"Имя: {user.get('in_bot_name', 'Не указано')}\n"
        f"Пол: {user.get('sex', 'Не указан')}\n"
        f"Возраст: {user.get('years', 'Не указан')}\n"
        f"Статус: {status}"
    )

    # Отправка фото профиля если есть
    photos = await message.bot.get_user_profile_photos(message.from_user.id, limit=1)
    if photos.total_count > 0:
        photo = photos.photos[0][-1]
        file = await message.bot.download(photo.file_id)
        file_bytes = BytesIO(file.read())
        await message.answer_photo(
            BufferedInputFile(file_bytes.read(), filename="profile.jpg"),
            caption=profile_text,
            reply_markup=kb.myanket_menu
        )
    else:
        await message.answer(profile_text, reply_markup=kb.myanket_menu)


@router.message(F.text == 'Изменить статус 🕐')
async def change_status(message: Message):
    await message.answer(
        "Выберите статус анкеты:",
        reply_markup=kb.status_keyboard()
    )

@router.callback_query(F.data.in_(["enable_profile", "disable_profile"]))
async def update_status(callback: CallbackQuery):
    new_status = 1 if callback.data == "enable_profile" else 0
    success = await rq.update_user_status(callback.from_user.id, new_status)

    if success:
        await callback.message.delete()
        await callback.answer("Статус обновлен ✅")
    else:
        await callback.answer("❌ Ошибка обновления")




@router.message(F.text == 'Искать партнера 😏')
async def find_partner(message: Message):
        await message.answer("Функция в разработке!", reply_markup=kb.menu)




# ---------- Админка ----------
@router.message(F.text == 'Админ-панель 👑')
async def admin_panel(message: Message):
    if message.from_user.id == int(os.getenv('ADMIN_ID')):
        await message.answer("Добро пожаловать в админ-панель!", reply_markup=kb.admin)
    else:
        await message.answer("Недостаточно прав", reply_markup=kb.menu)


@router.message(F.text == 'К-во userов')
async def users_count(message: Message):
    if message.from_user.id == int(os.getenv('ADMIN_ID')):
        count = await rq.get_users_count()
        await message.answer(f"Всего пользователей: {count}", reply_markup=kb.admin)
    else:
        await message.answer("Недостаточно прав", reply_markup=kb.menu)


# ---------- Навигация ----------
@router.message(F.text == 'Назад 👈')
async def back_to_menu(message: Message):
    if message.from_user.id == int(os.getenv('ADMIN_ID')):
        await message.answer("Главное меню", reply_markup=kb.admin_menu)
    else:
        await message.answer("Главное меню", reply_markup=kb.menu)